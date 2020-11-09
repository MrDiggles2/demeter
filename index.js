import express from 'express';
import * as sqlite from 'sqlite';
import sqlite3 from 'sqlite3';
import mqtt from 'mqtt';

(async() => {
    const db = await sqlite.open({
        filename: './database.sqlite',
        driver: sqlite3.Database
    });

    await db.migrate();

    const mqttClient  = mqtt.connect('mqtt://raspberrypi.local:1883');

    mqttClient.on('connect', function () {
        mqttClient.subscribe('$SYS/demeter/readings/+/#', function (err, granted) {
            if (err) {
                console.error(`Failed to subcribe: ${err}`);
            } else {
                console.log(`Sucessfully subscribed: ${JSON.stringify(granted)}`);
            }
        })
    });

    mqttClient.on('message', function (topic, message) {
        const regex = /\$SYS\/demeter\/readings\/(?<name>.+)\/(?<field>.+)/;
        const results = regex.exec(topic);

        if (results && results.groups) {
            const { name, field } = results.groups;
            test(name, field, message.toString());
        }
    });

    const data = {};

    function test(name, field, value) {
        if (!data[name]) {
            data[name] = {};
        }

        data[name][field] = value;

        record(data);
    }

    function record(data) {
        for(const name in data) {
            const {
                raw,
                percent,
                max,
                min
            } = data[name];

            if (!(raw && percent && max && min)) {
                continue;
            }

            data[name] = {};
            const addedAt = Math.floor(Date.now() / 1000);

            console.log("Logging reading: ", { name, raw, percent, max, min, addedAt });

            db.run(`
                INSERT INTO Reading (name, raw, percent, max, min, addedAt)
                VALUES ('${name}', ${raw}, ${percent}, ${max}, ${min}, ${addedAt})
            `);
        }
    }

    const app = express();
    const port = process.env.PORT || 3000;

    app.get('/readings', async (req, res, next) => {
        try {
            const posts = await db.all('SELECT * FROM Reading ORDER BY addedAt DESC LIMIT 10');
            res.send(posts);
        } catch (err) {
            next(err);
        }
    });

    app.listen(port, () => {
        console.log(`Listening at http://localhost:${port}`)
    });
})();

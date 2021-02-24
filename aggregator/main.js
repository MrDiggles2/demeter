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

    const mqttClient = mqtt.connect('mqtt://raspberrypi.local:1883');

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
        const regex = /\$SYS\/demeter\/readings\/(?<name>.+)\/raw$/;
        const results = regex.exec(topic);

        if (!results || !results.groups) {
            return;
        }

        const { name } = results.groups;
        const addedAt = Math.floor(Date.now() / 1000);
        const rawValue = parseInt(message.toString());

        console.log("Logging reading: ", { name, rawValue, addedAt });

        db.run(`
            INSERT INTO Reading (name, raw, addedAt)
            VALUES ('${name}', ${rawValue}, ${addedAt})
        `);
    });

    const app = express();
    const port = process.env.PORT || 3000;

    app.get('/readings', async (req, res, next) => {
        const startUnix = req.query.start_unix ?? Date.now() / 1000 - 60 * 60 * 1000;
        const endUnix = req.query.end_unix ?? Date.now() / 1000;

        try {
            const posts = await db.all(`
                SELECT
                    id,
                    name,
                    raw,
                    addedAt
                FROM Reading
                WHERE addedAt >= ${startUnix} AND addedAt <= ${endUnix}
                ORDER BY addedAt DESC
                LIMIT 10
            `);

            // Get unique "name" fields from results for debuggin
            const includedSensors = Array.from(
                new Set(
                    posts.map(post => post.name)
                )
            );

            res.send({
                data: posts,
                meta: {
                    start_unix: startUnix,
                    end_unix: endUnix,
                    included_sensors: includedSensors
                }
            });

        } catch (err) {
            next(err);
        }
    });

    app.listen(port, () => {
        console.log(`Listening at http://localhost:${port}`)
    });
})();

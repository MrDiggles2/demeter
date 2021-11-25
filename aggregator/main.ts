import express from 'express';
import * as sqlite from 'sqlite';
import sqlite3 from 'sqlite3';
import mqtt from 'mqtt';
import cors from 'cors';
import morgan from 'morgan';
import { DBService } from './db.service';

const TIMEOUT_MS = 24 *  60 * 60 * 1000;

(async() => {
    const db = await sqlite.open({
        filename: './database.sqlite',
        driver: sqlite3.Database
    });
    const dbService = new DBService(db);
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

    mqttClient.on('message', async (topic, message) => {
        const regex = /\$SYS\/demeter\/readings\/(?<name>.+)\/raw$/;
        const results = regex.exec(topic);

        if (!results || !results.groups) {
            return;
        }

        const { name } = results.groups;
        const value = parseInt(message.toString());

        console.log("Logging reading: ", { name, value });

        await dbService.recordReading(name, value);
    });

    const app = express();
    const port = process.env.PORT || 3000;
    app.listen(port, () => {
        console.log(`Listening at http://localhost:${port}`)
    });
    app.use(cors());
    app.use(morgan('tiny'));

    app.use('/views', express.static('views'))

    app.get('/readings', async (req, res) => {
        const count = parseInt(req.query.count?.toString() ?? '10');
        const sensorNames = req.query.names?.toString().split(',') ?? [];

        const readings = await dbService.getReadings(sensorNames, count);

        res.send({
            data: readings.map(reading => ({
                ...reading,
                ts: new Date(reading.addedAt * 1000).toISOString()
            })),
            meta: { count }
        });
    });

    app.get('/status', async (req, res, next) => {
        const ignorePattern = req.query.ignorePattern?.toString() ?? null;

        try {
            const posts = await db.all(`
                SELECT r1.*
                FROM Reading r1
                LEFT OUTER JOIN Reading r2
                    ON r1.name = r2.name AND r1.id < r2.id
                WHERE r2.id IS null
            `);

            res.send({
                data: posts
                    .filter(post => {
                        const { name } = post;

                        if (ignorePattern === null) {
                            return true;
                        }

                        var regex = new RegExp(ignorePattern, 'g');
                        return !regex.test(name);
                    })
                    .map(post => {
                        const { name, raw, addedAt } = post;

                        return {
                            name,
                            // TODO: replace this with some kind of calculation based on "raw" later
                            value: Math.round(Math.random() * 80 + 20),
                            isAlive: (Date.now() - (addedAt * 1000)) < TIMEOUT_MS,
                        };
                    }
                ),
                meta: {
                    query: {
                        ignorePattern
                    }
                }
            });
        } catch (err) {
            next(err);
        }
    });
})();

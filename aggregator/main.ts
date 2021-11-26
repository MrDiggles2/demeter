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

    mqttClient.on('connect', () => {
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
        const statuses = await dbService.getSensorStatuses();

        res.send({
            data: statuses,
            metadata: { }
        });
    });

    app.post('/readings', async (req, res, next) => {

        const value = req.query.value?.toString() ?? null;
        const name = req.query.name?.toString() ?? null;

        if (name === null || value === null) {
            res.status(400);
            return res.send({ message: 'Invalid format' });
        } else {
            console.log("Logging reading: ", { name, value });
            await dbService.recordReading(name, parseInt(value));
            res.status(201);
            return res.send();
        }
    });
})();

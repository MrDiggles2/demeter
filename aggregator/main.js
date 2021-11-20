import express from 'express';
import * as sqlite from 'sqlite';
import sqlite3 from 'sqlite3';
import mqtt from 'mqtt';
import cors from 'cors';
import morgan from 'morgan';
import regression from 'regression';
import stdev from '@stdlib/stats-base-dists-logistic-stdev';
import stats from 'stats-lite';

const TIMEOUT_MS = 24 *  60 * 60 * 1000;

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

        console.log(`Logging reading: ${JSON.stringify({ name, rawValue, addedAt }, null, 4)}`);

        db.run(`
            INSERT INTO Reading (name, raw, addedAt)
            VALUES ('${name}', ${rawValue}, ${addedAt})
        `);
    });

    const app = express();
    const port = process.env.PORT || 3000;
    app.listen(port, () => {
        console.log(`Listening at http://localhost:${port}`)
    });
    app.use(cors());
    app.use(morgan('tiny'));

    app.use('/views', express.static('views'))

    app.get('/readings', async (req, res, next) => {
        const count = req.query.count ?? 10;

        try {
            const posts = await db.all(`
                SELECT
                    id,
                    name,
                    raw,
                    addedAt
                FROM Reading
                WHERE id < 4516 AND id > 3460
                ${req.query.names ? `AND name IN (${req.query.names.split(',').map(n => `'${n}'`).join(',')})` : ''}
                ORDER BY id DESC
                LIMIT ${count}
            `);

            // Get unique "name" fields from results for debuggin
            const includedSensors = Array.from(
                new Set(
                    posts.map(post => post.name)
                )
            );

            res.send({
                data: posts.map(post => {
                    return {
                        ...post,
                        ts: new Date(post.addedAt * 1000).toISOString()
                    };
                }),
                meta: {
                    query: {
                        count
                    },
                    includedSensors
                }
            });

        } catch (err) {
            next(err);
        }
    });

    app.get('/status', async (req, res, next) => {
        const ignorePattern = req.query.ignorePattern ?? null;

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

    app.get('/regression', async (req, res, next) => {
        
        const posts = await db.all(`
            select raw, addedAt from Reading
            where
                name = "pcb-test-w-220uF-new-resistors"
                AND id < 4516 and id > 3460
            order by id desc
        `);

        const filtered = posts.filter((_, i) => {
            return true;
        });

        const results = [];
        const isNumberWithinPercentOfNumber = (n1, n2, percentage) => percentage > Math.abs(Math.abs(n2 - n1) / n2) * 100.0;

        for (let i = 300; i < filtered.length; i++) {
            const set = [];
            for (let j = 0; j < 300; j++) {
                const before = filtered[i - j - 1];
                const current = filtered[i - j];
                const after = filtered[i - j + 1];

                if (!current) continue;

                if (
                    before && after
                    && isNumberWithinPercentOfNumber(before.raw, current.raw, 10)
                    && isNumberWithinPercentOfNumber(after.raw, current.raw, 10)
                ) {
                    set.push(current);
                }
            }

            const oldestDate = Math.min(...set.map(val => val.addedAt)) - 100;
            const points = set
                .map(val => [ (val.addedAt - oldestDate) / 60 / 60, val.raw ])
                .reverse();

            const stddev = stats.stdev(set
                .filter(p => p.raw !== null && !Number.isNaN(p.raw))
                .map(p => p.raw));
            // console.log(stddev);

            const result = regression.linear(
            // const result = regression.logarithmic(
                points,
                {
                    precision: 10,
                }
            );

            const [ a, b ] = result.equation;
            results.push({
                rate: a,
                // rate: b,
                // rate: stddev,
                tip: set[set.length - 1].raw,
                lastTs: (new Date(set[set.length - 1].addedAt * 1000)).toISOString()
            });
        };

        const filteredResults = results.filter(r => r.rate !== null && !Number.isNaN(r.rate) && r.rate < 1);

        res.send({
            tip: {
                max: Math.max(...filteredResults.map(r => r.tip)),
                min: Math.min(...filteredResults.map(r => r.tip)),
            },
            rate: {
                max: Math.max(...filteredResults.map(r => r.rate)),
                min: Math.min(...filteredResults.map(r => r.rate))
            },
            data: filteredResults
        });
    });
})();



import * as path from 'path';
import * as sqlite from 'sqlite';
import sqlite3 from 'sqlite3';
import { DBService } from '../db.service';

describe('DB service', () => {

    let db: sqlite.Database;
    let dbService: DBService;

    beforeEach(async () => {
        if (db) {
            await db.close();
        }

        db = await sqlite.open({
            filename: ':memory:',
            driver: sqlite3.Database
        });

        await db.migrate({
            migrationsPath: path.join(__dirname, '..', 'migrations')
        });

        dbService = new DBService(db);
    });

    it('should record reading to work with an existing sensor', async () => {
        const name = 'existing';
        const value = 100;
        await db.run(`INSERT INTO Sensor (name, ignore) VALUES (?, ?)`, name, 0);

        await dbService.recordReading(name, value);

        const records = await db.all(`
            SELECT * FROM Reading
            LEFT JOIN Sensor ON Reading.sensorId = Sensor.id
            WHERE name = ? AND raw = ?
        `, name, value);

        expect(records).toEqual(
            expect.arrayContaining([
                expect.objectContaining({
                    name,
                    raw: value,
                })
            ])
        );
    });

    it('should create a new sensor row when recording values', async () => {
        const name = 'existing';
        const value = 100;

        await dbService.recordReading(name, value);

        const records = await db.all(`
            SELECT * FROM Reading
            LEFT JOIN Sensor ON Reading.sensorId = Sensor.id
            WHERE name = ? AND raw = ?
        `, name, value);

        expect(records).toEqual(
            expect.arrayContaining([
                expect.objectContaining({
                    name,
                    raw: value,
                })
            ])
        );
    });
});
import * as sqlite from 'sqlite';
import regression from 'regression';

export interface Sensor {
    id: number;
    name: string;
    /**
     * 1 for true, 0 for false
     */
    ignore: number;
}

export interface SensorStatus {
    sensor: Sensor;
    latestReading: Reading;
    isAlive: boolean;
    moisturePercentage: number;
}

export interface Reading {
    id: number;
    sensorId: number;
    raw: number;
    addedAt: number;
}

export class DBService {
    constructor(private db: sqlite.Database) {}

    public async recordReading(name: string, value: number): Promise<void> {
        const sensor = await this.findOrCreateSensorByName(name);
        const addedAt = Math.floor(Date.now() / 1000);

        await this.db.run(
            `INSERT INTO Reading (sensorId, raw, addedAt) VALUES (?, ?, ?)`,
            sensor.id, value, addedAt
        );
    }

    public async getReadings(names: string[], count: number): Promise<Reading[]> {
        return this.db.all<Reading[]>(`
            SELECT
                Reading.id,
                sensorId
                raw,
                addedAt
            FROM Reading
            LEFT JOIN Sensor ON Sensor.id = Reading.sensorId
            ${names.length ? `WHERE Sensor.name IN (${names.map(n => `'${n}'`).join(',')})` : ''}
            ORDER BY Reading.id DESC
            LIMIT ${count}
        `);
    }

    public async getSensorStatuses(): Promise<SensorStatus[]> {
        const statuses: SensorStatus[] = [];

        const latestReadings = await this.db.all<Array<Reading>>(`
            SELECT r1.*
            FROM Reading r1
            LEFT OUTER JOIN Reading r2
                ON r1.sensorId = r2.sensorId AND r1.id < r2.id
            WHERE r2.id IS null
        `);

        for (const latestReading of latestReadings) {
            const sensor = await this.findSensorById(latestReading.sensorId);

            statuses.push({
                sensor,
                latestReading,
                isAlive: await this.isSensorAlive(sensor, latestReading),
                moisturePercentage: await this.calculateMoisturePercentage(sensor),
            });
        }

        return statuses;
    }

    private async isSensorAlive(sensor: Sensor, reading: Reading): Promise<boolean> {
        // Consider sensor dead if latest reading was more than a week ago
        return (Date.now() - (reading.addedAt * 1000)) > 7 * 24 * 60 * 60 * 1000;
    }

    private async calculateMoisturePercentage(sensor: Sensor): Promise<number> {
        // TODO
        return 50;
    }

    private async findSensorById(id: number): Promise<Sensor> {
        return await this.getOrFail<Sensor>(`SELECT * FROM Sensor WHERE id = ?`, id);
    }

    private async findOrCreateSensorByName(name: string): Promise<Sensor> {
        const sensor = await this.db.get<Sensor>(`SELECT * FROM Sensor WHERE name = ?`, name);

        if (sensor) {
            return sensor;
        }

        await this.db.run(`INSERT INTO Sensor (name, ignore) VALUES (?, ?)`, name, 0);
        return await this.getOrFail<Sensor>(`SELECT * FROM Sensor WHERE name = ?`, name);
    }

    private async getOrFail<T>(statement: string, ...args: any[]): Promise<T> {
        const result = await this.db.get<T>(statement, ...args);

        if (!result) {
            throw new Error(`No records found for "${statement}", ${JSON.stringify(args)}`);
        }

        return result;
    }
}

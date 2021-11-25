import * as sqlite from 'sqlite';

export interface Sensor {
    id: number;
    name: string;
    /**
     * 1 for true, 0 for false
     */
    ignore: number;
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

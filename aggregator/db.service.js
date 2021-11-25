export class DBService {
    constructor(db) {
        this.db = db;
    }

    async recordReading(name, value) {
        const sensor = await this.findOrCreateSensorByName(name);
        const addedAt = Math.floor(Date.now() / 1000);

        return this.db.run(
            `INSERT INTO Reading (sensorId, raw, addedAt) VALUES (?, ?, ?)`,
            sensor.id, value, addedAt
        );
    }

    async findOrCreateSensorByName(name) {
        const sensor = await this.db.get(`SELECT * FROM Sensor WHERE name = ?`, name);

        if (sensor) {
            return sensor;
        }

        await this.db.run(`INSERT INTO Sensor (name, ignore) VALUES (?, ?)`, name, 0);
        return await this.db.get(`SELECT * FROM Sensor WHERE name = ?`, name);
    }
}

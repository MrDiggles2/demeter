export class DBService {
    constructor(db) {
        this.db = db;
    }

    async recordReading(name, value) {
        const addedAt = Math.floor(Date.now() / 1000);

        return db.run(`
            INSERT INTO Reading (name, raw, addedAt)
            VALUES ('${name}', ${rawValue}, ${addedAt})
        `);
    }
}

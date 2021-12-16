import * as sqlite from 'sqlite';

export const getMigrations = async (db: sqlite.Database): Promise<string[]> => {
    const migrations = await db.all('SELECT name FROM migrations');
    return migrations.map(m => m.name);
};
import * as path from 'path';
import * as sqlite from 'sqlite';
import sqlite3 from 'sqlite3';
import { getMigrations } from './shared';

(async () => {
    try {
        const db = await sqlite.open({
            filename: path.join(__dirname, '..', 'database.sqlite'),
            driver: sqlite3.Database
        });

        console.log(`Migrations before: `);
        (await getMigrations(db)).forEach(name => console.log(`    ${name}`));

        await db.migrate({
            table: 'migrations',
            migrationsPath: path.join(__dirname, '..', 'migrations')
        });

        console.log(`Migrations after: `);
        (await getMigrations(db)).forEach(name => console.log(`    ${name}`));

        console.log('\ndone');
    } catch (e) {
        console.error('Unable to run migrations. (Did you remember to turn off the application?)');
        console.error(e);
    }
})();
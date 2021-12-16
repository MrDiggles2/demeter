import * as path from 'path';
import * as sqlite from 'sqlite';
import fs from 'fs';
import sqlite3 from 'sqlite3';
import { getMigrations } from './shared';

(async () => {
    try {
        const db = await sqlite.open({
            filename: path.join(__dirname, '..', 'database.sqlite'),
            driver: sqlite3.Database
        });

        const completedMigrations = await getMigrations(db);
        const migrationFileNames = fs.readdirSync(path.join(__dirname, '..', 'migrations'));

        migrationFileNames
            .map(fileName => fileName.split('.')[0].split('-').slice(1).join('-'))
            .forEach(migrationName => {
                if (completedMigrations.includes(migrationName)) {
                    console.log(`  [\u2713] ${migrationName}`);
                } else {
                    console.log(`  [ ] ${migrationName}`);
                }
            });

        console.log('\ndone');

    } catch (e) {
        console.error('Unable to list migrations. (Did you remember to turn off the application?)');
        console.error(e);
    }
})();
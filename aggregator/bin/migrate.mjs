#! /usr/bin/env node

import * as path from 'path';
import { fileURLToPath } from 'url';
import * as sqlite from 'sqlite';
import sqlite3 from 'sqlite3';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

(async () => {
    try {
        const db = await sqlite.open({
            filename: path.join(__dirname, '..', 'database.sqlite'),
            driver: sqlite3.Database
        });

        await db.migrate({
            migrationsPath: path.join(__dirname, '..', 'migrations')
        });
    } catch (e) {
        console.error('Unable to run migrations. (Did you remember to turn off the application?)');
        console.error(e);
    }
})();
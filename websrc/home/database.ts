import { MTY_Alloc, mty_b64_to_buf, mty_buf_to_b64, mty_mem, MTY_SetUint32, MTY_StrToJS, MTY_Free } from "./matoya";
import { openDB, deleteDB, wrap, unwrap } from 'idb';

let database;
let files = {};

const request = window.indexedDB.open('Junie', 1);

//Check if a schema update is required
request.onupgradeneeded = event =>
{
    const database = (event.target as any).result;

    if (event.oldVersion < 1) {
        database.createObjectStore('files', { keyPath: 'path', unique: true });
        database.createObjectStore('games', { keyPath: 'path', unique: true });
    }
};

//Retrieve all files in memory
request.onsuccess = event => 
{
    database = (event.target as any).result;

    const transaction = database.transaction('files', 'readwrite');
    const store       = transaction.objectStore('files');

    store.getAll().onsuccess = event => {
        for (const file of event.target.result) {
            files[file.path] = mty_b64_to_buf(file.data);
        }
    }

    //Begin: compatibility with older versions
    for (let i = 0; i < localStorage.length; ++i) {
        const path = localStorage.key(i);
        const data = localStorage.getItem(path);

        const entry = { path: `/${path}`, data: data };

        store.put(entry);

        files[entry.path] = mty_b64_to_buf(entry.data);
    }

    localStorage.clear();
    //End: compatibility with older versions
};

//Read a file from memory storage
let fileBuffer = null;
export function JUN_ReadFile(path, length) {    
    console.error("JUN_ReadFile path name:", MTY_StrToJS(path), length, files);
    const file = files[MTY_StrToJS(path)];
    if (!file)
        return null;

    if (fileBuffer) {
        window["oldFileBuffer"] = fileBuffer;
        console.log("Removing Old file Buffer from memory later");
        // MTY_Free(window"oldFileBuffer"]);
    }
    fileBuffer = MTY_Alloc(file.length, 1);
    const view = new Uint8Array(mty_mem(), fileBuffer, file.length);
    view.set(file);

    MTY_SetUint32(length, file.length);

    return fileBuffer;
}

//Write a file to the database
export function JUN_WriteFile(cpath, cdata, length) {
    const transaction = database.transaction('files', 'readwrite');
    const store       = transaction.objectStore('files');

    const buffer = new Uint8Array(length);
    const data   = new Uint8Array(mty_mem(), cdata, length);
    buffer.set(data);

    const path = MTY_StrToJS(cpath)

    files[path] = data;

    store.put({ path: path, data: mty_buf_to_b64(data) });
}

export async function JUN_WriteFileFromJS(file, system) {
    const fileContent = await file.arrayBuffer();
    const transaction = database.transaction('games', 'readwrite');
    const store       = transaction.objectStore('games');
    store.put({ path: system+"/"+file.path, data: fileContent });
    // files[file.path] = fileContent;
}

export async function RE_ReadFileFromJS(file_path) {
    let db = await openDB('Junie', 1);

    let tx = db.transaction('games', 'readonly')
    let store = tx.objectStore('games')
    return await store.get(file_path);
}

export async function RE_getAllLocalGames() {
    let db = await openDB('Junie', 1);

    let tx = db.transaction('games', 'readonly')
    let store = tx.objectStore('games')
    return await store.getAll();
}
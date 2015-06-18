#!/usr/bin/env python3

import sqlite3
import sys


def tobytes(string):
    return bytes(map(lambda i: int(i,16), string.split()))

def insert(cur, id, name):
    cur.execute('insert into stations(id, name) values (?, ?)', (tobytes(id), name))


if __name__ == '__main__':
    with open('stations.txt', 'rt') as f:
        with sqlite3.connect('./ids.sqlite') as con:
            cur = con.cursor()
            for line in f:
                id = line[:5]
                if not id.strip(): continue
                name = line[5:].strip()
                cur.execute('select id from stations where id=?', (tobytes(id),))
                if not list(cur):
                    print('Inserting ({0}, {1})'.format(id, name))
                    insert(cur, id, name)

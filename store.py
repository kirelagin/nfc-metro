#!/usr/bin/env python3

import argparse
import sqlite3


def insert(cur, id, name):
    id = bytes(map(lambda i: int(i,16), id.split()))
    cur.execute('insert into stations(id, name) values (?, ?)', (id, name))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('id', metavar='id', type=str)
    parser.add_argument('name', metavar='name', type=str)
    args = parser.parse_args()

    with sqlite3.connect('./ids.sqlite') as con:
        cur = con.cursor()
        insert(cur, args.id, args.name)

#!/bin/python
# Copyright by Potato Engine contributors.  See accompanying License.txt for
# copyright details.
import sys
import json
import argparse
import os
from os import path
from datetime import datetime

class TypeDatabase:
    """Collection of all known types"""
    def __init__(self):
        self.__types = {}
        self.__exports = []

    def load_from_json(self, doc):
        print(doc)
        for key,type_json in doc['types'].items():
            type = TypeDefinition(self)
            type.load_from_json(type_json)
            self.__types[type.name] = type

class TypeDefinition:
    """User-friendly wrapper of a type-definition in the type database"""
    def __init__(self, db):
        self.__db = db

    def load_from_json(self, json):
        self.name = json['name']

def generate(db, file, input_name):
    """Executor for the brunt of code generation"""

    file.write(f"""
// --- GENERATED FILE ----
// - Do not edit this file
// - Generated on {datetime.utcnow()} UTC
// - Generated from {path.basename(file.name)}
""")

    file.write(f"""
namespace up {{
    inline namespace schema {{
    }} // namespace up::schema
}} // namespace up
""")
    pass

def main(argv):
    """Main entrypoint for the program"""
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input', type=argparse.FileType(mode='r', encoding='utf-8'), required=True, help="Input schema json")
    parser.add_argument('-o', '--output', type=str, help="Target output file")
    args = parser.parse_args(argv[1:])

    doc = json.load(args.input)
    args.input.close()

    db = TypeDatabase()
    db.load_from_json(doc)

    if args.output is None:
        generate(db=db, file=sys.stdout, input_name=args.input.name)
    else:
        parent_path = path.dirname(args.output)
        os.makedirs(name=parent_path, exist_ok=True)
        with open(args.output, 'wt', encoding='utf-8') as file:
            generate(db=db, file=file, input_name=args.input.name)

    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv))

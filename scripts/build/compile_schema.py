#!/bin/python
# Copyright by Potato Engine contributors.  See accompanying License.txt for
# copyright details.
import sys
import json
import argparse
import os
from os import path
from datetime import datetime
import type_info

class Context:
    """Complete context used by a generator"""
    def __init__(self, db, output, input_name):
        self.__db = db
        self.__output = output
        self.__input_name = input_name

    @property
    def db(self):
        return self.__db

    @property
    def input_name(self):
        return self.__input_name

    @property
    def output(self):
        return self.__output

    def print(self, *args):
        self.__output.write(''.join(*args))

def generate_file_prefix(ctx: Context):
    """Writes the header at the top of a generated C++ file"""
    ctx.print(f"""
// --- GENERATED FILE ----
// - Do not edit this file
// - Generated on {datetime.utcnow()} UTC
// - Generated from {path.basename(ctx.input_name)}
""")

def generate_header_types(ctx: Context):
    """Generates the type definitions for types"""
    for name, type in ctx.db.exports.items():
        ctx.print(f"struct {type.cxxname} {{\n")
        for field in type.fields_ordered:
            ctx.print(f"    {field.cxxtype} {field.cxxname};\n")
        ctx.print("};\n")

def generate_header_reflex(ctx: Context):
    """Generates the reflex definitions for types"""
    for name, type in ctx.db.exports.items():
        ctx.print(f"UP_REFLECT_TYPE({type.cxxname}) {{\n")
        for field in type.fields_ordered:
            ctx.print(f'    reflect("{field.name}", &Type::{field.cxxname});\n')
        ctx.print("}\n")

def generate_header_json_parse_decl(ctx: Context):
    """Generates json parsing function declarations for types"""
    for name, type in ctx.db.exports.items():
        if not type.has_annotation("serialize"):
            continue

        ctx.print(f"void from_json(nlohmann::json const& root, {type.cxxname}& value);\n")

def generate_header_json_parse_impl(ctx: Context):
    """Generates json parsing function definition for types"""
    for name, type in ctx.db.exports.items():
        if not type.has_annotation("serialize"):
            continue

        ctx.print(f"void from_json(nlohmann::json const& root, {type.cxxname}& value) {{\n")
        ctx.print("""
    if (!root.is_object()) {
        return;
    }
""")
        for field in type.fields_ordered:
            json_name = field.get_annotation_field_or("json", "name", field.name)

            if field.is_array:
                ctx.print(f'''
    if (auto it = root.find("{json_name}"); it != root.end() && it->is_array()) {{
        for (auto const& child : *it) {{
            child.get_to(value.{field.cxxname}.emplace_back());
        }}
    }}
''')
            else:
                ctx.print(f'''
    if (auto it = root.find("{json_name}"); it != root.end()) {{
         it->get_to(value.{field.cxxname});
    }}
''')

            if field.has_default:
                ctx.print("    else {\n        value.{field.cxxname} = {field.default_or(None)};\n    }\n")

        ctx.print("}\n")

def generate_header(ctx: Context):
    """Generate contents of header file"""

    generate_file_prefix(ctx)

    ctx.print(f"""
#pragma once
#include "potato/spud/string.h"
#include "potato/spud/vector.h"
#include "potato/reflex/reflect.h"
#include "potato/runtime/json.h"
namespace up {{
    inline namespace schema {{
""")

    generate_header_types(ctx)
    generate_header_reflex(ctx)
    generate_header_json_parse_decl(ctx)

    ctx.print(f"""
    }} // namespace up::schema
}} // namespace up
""")

def generate_source(ctx: Context):
    """Generate contents of a source file"""

    generate_file_prefix(ctx)
    ctx.print(f"""
#include "{ctx.db.module}_schema.h"
#include <nlohmann/json.hpp>
namespace up {{
    inline namespace schema {{
""")

    generate_header_json_parse_impl(ctx)

    ctx.print(f"""
    }} // namespace up::schema
}} // namespace up
""")

generators = {
    'header': generate_header,
    'source': generate_source
}

def main(argv):
    """Main entrypoint for the program"""
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input', type=argparse.FileType(mode='r', encoding='utf-8'), required=True, help="Input schema json")
    parser.add_argument('-o', '--output', type=str, help="Target output file")
    parser.add_argument('-G', '--generator', type=str, help="Generation mode", choices=["header", "source"])
    args = parser.parse_args(argv[1:])

    doc = json.load(args.input)
    args.input.close()

    generator = generators[args.generator]

    db = type_info.TypeDatabase()
    db.load_from_json(doc)

    output = sys.stdout
    if args.output is not None:
        parent_path = path.dirname(args.output)
        os.makedirs(name=parent_path, exist_ok=True)
        output = open(args.output, 'wt', encoding='utf-8')

    ctx = Context(db=db, output=output, input_name=args.input.name)

    rs = generator(ctx)
    if rs is None or rs == 0:
        return 0
    else:
        return 1

if __name__ == '__main__':
    sys.exit(main(sys.argv))
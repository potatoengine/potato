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
    def __init__(self, db, output, input_name, library):
        self.__db = db
        self.__output = output
        self.__input_name = input_name
        self.__library = library

    @property
    def db(self):
        return self.__db

    @property
    def input_name(self):
        return self.__input_name

    @property
    def library(self):
        return self.__library

    @property
    def output(self):
        return self.__output

    def print(self, *args):
        self.__output.write(''.join(args))

def generate_file_prefix(ctx: Context):
    """Writes the header at the top of a generated C++ file"""
    ctx.print(f"""
// --- GENERATED FILE ----
// - Do not edit this file
// - Generated on {datetime.utcnow()} UTC
// - Generated from {path.basename(ctx.input_name)}
""")

def cxxnamespace(type: type_info.TypeBase, namespace: str='up::schema'):
    """Returns the desired namespace for a type"""
    return type.get_annotation_field_or('cxxnamespace', 'ns', namespace if not type.has_annotation('Component') else 'up::components')

def qualified_cxxname(type: type_info.TypeBase, namespace: str='up::schema'):
    """Calculates the qualified name for types"""
    cxxname = type.cxxname
    cxxns = cxxnamespace(type, namespace)
    return cxxname if '::' in cxxname or type.module == '$core' else f'{cxxns}::{cxxname}'

def cxxvalue(value):
    if value is True:
        return 'true'
    if value is False:
        return 'false'
    if isinstance(value, str):
        return f'"{value}"' # FIXME: escapes
    return str(value)

def generate_header_types(ctx: Context):
    """Generates the type definitions for types"""
    for _, type in ctx.db.exports.items():
        if type.has_annotation('ignore'):
            continue
        if type.kind == type_info.TypeKind.OPAQUE:
            continue
        if type.has_annotation('cxximport'):
            continue

        cxxns = cxxnamespace(type)

        ctx.print(f'namespace {cxxns} {{\n')

        ctx.print(f'    struct {type.cxxname} ')
        if type.kind == type_info.TypeKind.ATTRIBUTE:
            ctx.print(': reflex::SchemaAttribute ')
        ctx.print('{\n')
        for field in type.fields_ordered:
            ctx.print(f"        {field.cxxtype} {field.cxxname}")
            if field.has_default:
                ctx.print(f' = {cxxvalue(field.default_or(None))}')
            ctx.print(";\n")
        ctx.print("    };\n")

        ctx.print('}\n')

def generate_header_schemas(ctx: Context):
    """Generates the Schema declarations for types"""
    ctx.print("namespace up::reflex {\n")

    for type in ctx.db.exports.values():
        if type.has_annotation('ignore'):
            continue

        qual_name = qualified_cxxname(type=type)

        ctx.print("    template <>\n")
        ctx.print(f"    struct TypeHolder<{qual_name}> {{\n")
        ctx.print(f"        UP_{ctx.library.upper()}_API static TypeInfo const& get() noexcept;\n")
        ctx.print("    };\n")
        ctx.print("    template <>\n")
        ctx.print(f"    struct SchemaHolder<{qual_name}> {{\n")
        ctx.print(f"        UP_{ctx.library.upper()}_API static Schema const& get() noexcept;\n")
        ctx.print("    };\n")
    
    ctx.print('}\n')

def generate_header_json_parse_decl(ctx: Context):
    """Generates json parsing function declarations for types"""
    for type in ctx.db.exports.values():
        if not type.has_annotation("serialize"):
            continue
        if type.kind == type_info.TypeKind.OPAQUE:
            continue

        cxxns = cxxnamespace(type)
        ctx.print(f'namespace {cxxns} {{\n')

        ctx.print(f"    UP_{ctx.library.upper()}_API void from_json(nlohmann::json const& root, {type.cxxname}& value);\n")

        ctx.print('}\n')

def generate_impl_json_parse(ctx: Context):
    """Generates json parsing function definition for types"""
    for type in ctx.db.exports.values():
        if type.has_annotation('ignore'):
            continue
        if not type.has_annotation("serialize"):
            continue
        if type.kind == type_info.TypeKind.OPAQUE:
            continue

        ctx.print(f"void up::schema::from_json(nlohmann::json const& root, {type.cxxname}& value) {{\n")
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
                ctx.print(f'    else {{\n        value.{field.cxxname} = {cxxvalue(field.default_or(None))};\n    }}\n')

        ctx.print('}\n')

def generate_impl_annotations(ctx: Context, name: str, entity: type_info.AnnotationsBase):
    """Generates metadata for an annotation"""
    locals = []
    for annotation in entity.annotations:
        attr = annotation.type
        local_name = f'attr_{name}_{attr.name}'
        locals.append(local_name)

        ctx.print(f'    static const {attr.cxxname} {local_name}{{')
        for field, value in zip(attr.fields_ordered, annotation.values):
            ctx.print(f'.{field.cxxname} = {cxxvalue(value)}, ')
        ctx.print('};\n')

    if len(locals) != 0:
        ctx.print(f'    static const SchemaAnnotation {name}_annotations[] = {{\n')
        for local in locals:
            ctx.print(f'        {{ .type = &getTypeInfo<decltype({local})>(), .attr = &{local} }},\n')
        ctx.print('    };\n')
    else:
        ctx.print(f'    static const view<SchemaAnnotation> {name}_annotations;\n')

def generate_impl_schemas(ctx: Context):
    """Generates the Schema definitions for types"""
    for type in ctx.db.exports.values():
        if type.has_annotation('ignore'):
            continue

        qual_name = qualified_cxxname(type=type)

        ctx.print(f"up::reflex::TypeInfo const& up::reflex::TypeHolder<{qual_name}>::get() noexcept {{\n")
        ctx.print('    using namespace up::schema;\n')
        ctx.print(f'    static const TypeInfo info = makeTypeInfo<{qual_name}>("{type.name}", &getSchema<{qual_name}>());\n')
        ctx.print('    return info;\n')
        ctx.print("}\n")

        if type.kind == type_info.TypeKind.OPAQUE:
            continue
        ctx.print(f"up::reflex::Schema const& up::reflex::SchemaHolder<{qual_name}>::get() noexcept {{\n")
        ctx.print('    using namespace up::schema;\n')

        generate_impl_annotations(ctx, type.name, type)
        for field in type.fields_ordered:
            generate_impl_annotations(ctx, f'{type.name}_{field.name}', field)

        if len(type.fields_ordered) != 0:
            ctx.print('    static const SchemaField fields[] = {\n')
            for field in type.fields_ordered:
                ctx.print(f'        SchemaField{{.name = "{field.name}", .schema = &getSchema<{field.cxxtype}>(), .offset = offsetof({qual_name}, {field.cxxname}), .annotations = {type.name}_{field.name}_annotations}},\n')
            ctx.print('    };\n')
        else:
            ctx.print('    static constexpr view<SchemaField> fields;\n')
        ctx.print(f'    static const Schema schema = {{.name = "{type.name}", .primitive = up::reflex::SchemaPrimitive::Object, .fields = fields, .annotations = {type.name}_annotations}};\n')
        ctx.print('    return schema;\n')
        ctx.print("}\n")

def generate_header(ctx: Context):
    """Generate contents of header file"""

    generate_file_prefix(ctx)

    ctx.print(f"""
#pragma once
#include "potato/spud/string.h"
#include "potato/spud/vector.h"
#include "potato/reflex/schema.h"
#include "potato/reflex/type.h"
#include "potato/runtime/json.h"
#include "potato/{ctx.library}/_export.h"
""")

    for imported in ctx.db.imports:
        ctx.print(f'#include "{imported}_schema.h"\n')

    for type in ctx.db.types.values():
        header = type.get_annotation_field_or('cxximport', 'header', None)
        if header is not None:
            ctx.print(f'#include "{header}"\n')

    generate_header_types(ctx)
    generate_header_json_parse_decl(ctx)
    generate_header_schemas(ctx)

def generate_source(ctx: Context):
    """Generate contents of a source file"""

    generate_file_prefix(ctx)
    ctx.print(f"""
#include "{ctx.db.module}_schema.h"
#include <nlohmann/json.hpp>
""")

    generate_impl_json_parse(ctx)
    generate_impl_schemas(ctx)

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
    parser.add_argument('-L', '--library', type=str, required=True, help="Name of C++ library")
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

    ctx = Context(db=db, output=output, input_name=args.input.name, library=args.library)

    rs = generator(ctx)
    if rs is None or rs == 0:
        return 0
    else:
        return 1

if __name__ == '__main__':
    sys.exit(main(sys.argv))

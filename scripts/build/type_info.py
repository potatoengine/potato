# Copyright by Potato Engine contributors.  See accompanying License.txt for
# copyright details.
from enum import Enum

class TypeKind(Enum):
    """Type kinds differentiate declarations such as structs and enums"""

    ATTRIBUTE = "attribute"
    ALIAS = "alias"
    STRUCT = "struct"
    ENUM = "enum"
    SIMPLE = "simple"
    POINTER = "pointer"
    ARRAY = "array"
    GENERIC = "generic"
    SPECIALIZED = "specialized"

class Location:
    """Source location"""
    def __init__(self, filename, line = None, column = None):
        self.filename = filename
        self.line = line
        self.column = column

    @classmethod
    def empty(cls):
        return Location('')

    def __str__(self):
        if self.line is None:
            return self.filename
        elif self.column is None:
            return f'{self.filename}({self.line})'
        else:
            return f'{self.filename}({self.line},{self.column})'

    @classmethod
    def from_json(cls, json):
        filename = json['filename'] if 'filename' in json else ''
        line = json['line'] if 'line' in json else None
        column = json['column'] if 'column' in json else None
        return Location(filename, line, column)

class Annotation:
    """A single annotation"""
    def __init__(self, type_name):
        self.__name = ''
        self.__type = None
        self.__type_name = type_name
        self.__args = []
        self.__argmap = {}
        self.__location = Location.empty()

    @property
    def type(self):
        return self.__type

    @property
    def location(self):
        return self.__location

    @property
    def values(self):
        return (arg for arg in self.__args)

    def value_or(self, key, default):
        if isinstance(key, str):
            return self.__argmap[key] if key in self.__argmap else default
        else:
            return self.__args[key]

    def load_from_json(self, json):
        self.__location = Location.from_json(json['location'])
        self.__args = [arg for arg in json['args']]

    def resolve(self, db):
        self.__type = db.type(self.__type_name)
        for index,field in enumerate(self.__type.fields_ordered):
            self.__argmap[field.name] = self.__args[index]

class AnnotationsBase:
    """Holder for annotations"""
    def __init__(self):
        self.__annotations = {}

    @property
    def annotations(self):
        return self.__annotations

    def __contains__(self, type):
        return self.has_annotation(type)

    def has_annotation(self, type):
        for anno in self.__annotations:
            if anno.type.name == type:
                return True
        return False

    def get_annotation(self, type, field):
        return self.get_annotation_field_or(type, field, None)

    def get_annotation_field_or(self, type, field, otherwise):
        for anno in self.__annotations:
            if anno.type.name == type:
                return anno.value_or(field, otherwise)
        return otherwise

    def load_from_json(self, json):
        def annotate(json):
            anno = Annotation(json['type'])
            anno.load_from_json(json)
            return anno
        self.__annotations = [annotate(anno) for anno in json['annotations']]

    def resolve(self, db):
        for anno in self.__annotations:
            anno.resolve(db)

class TypeDatabase:
    """Collection of all known types"""
    def __init__(self):
        self.__types = []
        self.__typemap = {}
        self.__module = ''
        self.__imports = []

    @property
    def module(self):
        return self.__module

    @property
    def types(self):
        return (type for type in self.__types)

    @property
    def imports(self):
        return (imp for imp in self.__imports)

    @property
    def exports(self):
        return (type for type in self.__types if type.module == self.__module)

    def type(self, name):
        return self.__typemap[name]

    def load_from_json(self, doc):
        module = doc['module']

        AnnotationsBase.load_from_json(self, module)

        self.__module = module['name']
        self.__imports = [mod['name'] for mod in module['imports']]
        for type_json in doc['types']:
            qualified = type_json['qualified']
            kind = type_json['kind']
            if kind == TypeKind.STRUCT.value:
                type = TypeStruct(self, qualified=qualified)
            elif kind == TypeKind.ATTRIBUTE.value:
                type = TypeAttribute(self, qualified=qualified)
            elif kind == TypeKind.SIMPLE.value:
                type = TypeSimple(self, qualified=qualified)
            elif kind == TypeKind.ALIAS.value:
                type = TypeAlias(self, qualified=qualified)
            elif kind == TypeKind.ENUM.value:
                type = TypeEnum(self, qualified=qualified)
            elif kind == TypeKind.POINTER.value:
                type = TypePointer(self, qualified=qualified)
            elif kind == TypeKind.ARRAY.value:
                type = TypeArray(self, qualified=qualified)
            elif kind == TypeKind.GENERIC.value:
                type = TypeGeneric(self, qualified=qualified)
            elif kind == TypeKind.SPECIALIZED.value:
                type = TypeSpecialized(self, qualified=qualified)
            else:
                raise Exception(f'Unknown kind {kind}')
            self.__types.append(type)
            self.__typemap[type.qualified] = type
            type.load_from_json(type_json)

        self.__resolve_types()

    def __resolve_types(self):
        for type in self.__types:
            type.resolve(self)

class TypeBase(AnnotationsBase):
    """User-friendly wrapper of a type-definition in the type database"""
    __builtin_types = ['char', 'float', 'double']

    def __init__(self, db, qualified):
        self.__db = db
        self.__module = db.module
        self.__name = ''
        self.__qualified = qualified
        self.__base_type_name = ''
        self.__base_type = None
        self.__location = Location.empty()

    @property
    def location(self):
        return self.__location

    @property
    def name(self):
        return self.__name

    @property
    def qualified(self):
        return self.__qualified

    @property
    def module(self):
        return self.__module

    @property
    def kind(self):
        return None

    @property
    def exported(self):
        return self.__module == self.__db.module

    @property
    def builtin(self):
        return self.__module == '@core'

    @property
    def cxxname(self):
        return self.get_annotation_field_or('cxxname', 'id', self.get_annotation_field_or('cxximport', 'id', self.__name))

    @property
    def base(self):
        return self.__base_type

    def load_from_json(self, json):
        AnnotationsBase.load_from_json(self, json)
        self.__location = Location.from_json(json['location'])
        self.__name = json['name']
        self.__base_type_name = json['base'] if 'base' in json and json['base'] is not None else ''
        self.__module = json['module']

    def resolve(self, db):
        if self.__base_type_name != '':
            self.__base_type = db.type(self.__base_type_name)
        AnnotationsBase.resolve(self, db)

class TypeSimple(TypeBase):
    """User-friendly wrapper of a simple type"""
    @property
    def kind(self):
        return TypeKind.SIMPLE

class TypeAlias(TypeBase):
    """User-friendly wrapper of an alias"""
    def __init__(self, db, qualified):
        TypeBase.__init__(self, db, qualified)
        self.__ref_name = ''
        self.__ref = None

    @property
    def kind(self):
        return TypeKind.ALIAS

    @property
    def ref(self):
        return self.__ref

    def load_from_json(self, json):
        TypeBase.load_from_json(self, json)
        if 'refType' in json:
            self.__ref_name = json['refType']

    def resolve(self, db):
        TypeBase.resolve(self, db)
        if self.__ref_name != '':
            self.__ref = db.type(self.__ref_name)

class TypePointer(TypeBase):
    """User-friendly wrapper of a pointer type"""
    @property
    def kind(self):
        return TypeKind.POINTER

class TypeArray(TypeBase):
    """User-friendly wrapper of an array type"""
    def __init__(self, db, qualified):
        TypeBase.__init__(self, db, qualified)
        self.__of_type_name = ''
        self.__of_type = None

    @property
    def kind(self):
        return TypeKind.ARRAY

    @property
    def of(self):
        return self.__of_type

    def load_from_json(self, json):
        TypeBase.load_from_json(self, json)
        self.__of_type_name = json['refType']

    def resolve(self, db):
        TypeBase.resolve(self, db)
        if self.__of_type_name != '':
            self.__of_type = db.type(self.__of_type_name)

class TypeStruct(TypeBase):
    """User-friendly wrapper of a struct definition"""
    def __init__(self, db, qualified):
        TypeBase.__init__(self, db, qualified)
        self.__fields = []
        self.__fieldmap = {}
        self.__generics = []

    @property
    def kind(self):
        return TypeKind.STRUCT

    @property
    def fields(self):
        return self.__fieldmap

    @property
    def fields_ordered(self):
        return self.__fields

    @property
    def generics(self):
        return [gen for gen in self.__generics]

    def load_from_json(self, json):
        TypeBase.load_from_json(self, json)
        for field_json in json['fields']:
            key = field_json['name']
            field = TypeField(owner=self, name=key)
            field.load_from_json(field_json)
            self.__fields.append(field)
            self.__fieldmap[key] = field
        if 'generics' in json:
            for generic in json['generics']:
                self.__generics.append(generic)

    def resolve(self, db):
        TypeBase.resolve(self, db)
        for field in self.__fields:
            field.resolve(db)

class TypeEnum(TypeBase):
    """User-friendly wrapper of an enum definition"""

    class EnumItem(object):
        """Individual enumeration item"""
        def __init__(self, name, value):
            self.name = name
            self.value = value

    def __init__(self, db, qualified):
        TypeBase.__init__(self, db, qualified)
        self.__items = []
        self.__itemmap = {}

    @property
    def kind(self):
        return TypeKind.ENUM

    def value_or(self, key, otherwise=None):
        return self.__itemmap[key].value if key in self.__itemmap else otherwise

    @property
    def values(self):
        return (item.value for item in self.__items)

    @property
    def names(self):
        return (item.name for item in self.__items)

    def load_from_json(self, json):
        TypeBase.load_from_json(self, json)
        for item_json in json['items']:
            item = TypeEnum.EnumItem(item_json['name'], item_json['value'])
            self.__items.append(item)
            self.__itemmap[item.name] = item

class TypeAttribute(TypeStruct):
    """User-friendly wrapper for an attribute struct"""
    @property
    def kind(self):
        return TypeKind.ATTRIBUTE

class TypeGeneric(TypeBase):
    """User-friendly wrapper for a generic type placeholder"""
    @property
    def kind(self):
        return TypeKind.GENERIC

class TypeSpecialized(TypeBase):
    """User-friendly wrapper of an specialized type definition"""

    def __init__(self, db, qualified):
        TypeBase.__init__(self, db, qualified)
        self.__ref = None
        self.__ref_name = None
        self.__params = []
        self.__param_names = []

    @property
    def kind(self):
        return TypeKind.SPECIALIZED

    @property
    def ref(self):
        return self.__ref

    @property
    def params(self):
        return (param for param in self.__params)

    def load_from_json(self, json):
        TypeBase.load_from_json(self, json)
        self.__ref_name = json['refType']
        for param_name in json['typeParams']:
            self.__param_names.append(param_name)

    def resolve(self, db):
        self.__ref = db.type(self.__ref_name)
        for param_name in self.__param_names:
            self.__params.append(db.type(param_name))

class TypeField(AnnotationsBase):
    """User-friendly wrapper for a field definition of a type"""
    def __init__(self, owner, name):
        self.__owner = owner
        self.__name = name
        self.__type_name = ''
        self.__type = None
        self.__default = None
        self.__has_default = False

    @property
    def name(self):
        return self.__name

    @property
    def type(self):
        return self.__type

    @property
    def is_array(self):
        return self.type.kind == TypeKind.ARRAY

    def default_or(self, otherwise):
        if self.__has_default:
            return self.__default
        else:
            return otherwise

    @property
    def has_default(self):
        return self.__has_default

    def load_from_json(self, json):
        AnnotationsBase.load_from_json(self, json)
        type = json['type']
        if isinstance(type, str):
            self.__type_name = json['type']
        if 'default' in json:
            self.__default = json['default']
            self.__has_default = True

    def resolve(self, db):
        self.__type = db.type(self.__type_name)
        AnnotationsBase.resolve(self, db)

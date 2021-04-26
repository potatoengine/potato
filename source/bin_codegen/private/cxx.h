// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "schema.h"

#include <ostream>
#include <string_view>

namespace cxx {
    struct Ident {
        std::string_view ident;

        friend std::ostream& operator<<(std::ostream& os, Ident const&& self) { return os << self.ident; }
    };

    struct TypeNamespacePrefix {
        schema::TypeBase const* type = nullptr;

        friend std::ostream& operator<<(std::ostream& os, TypeNamespacePrefix const&& self) {
            using namespace std::literals;

            if (auto const cxxns = getAnnotationArg(self.type->annotations, "cxxnamespace"); cxxns) {
                return os << schema::valueCast<std::string_view>(*cxxns) << "::";
            }
            if (self.type->kind == schema::TypeKind::Attribute) {
                return os << "schema::attribute::";
            }
            if (self.type->qualifiedName == "int"sv || self.type->qualifiedName == "float"sv ||
                self.type->qualifiedName == "bool"sv) {
                return os;
            }
            if (self.type->qualifiedName == "string"sv) {
                return os << "up::";
            }
            return os << "schema::";
        }
    };

    struct QualifiedName {
        schema::TypeBase const* type = nullptr;

        friend std::ostream& operator<<(std::ostream& os, QualifiedName const&& self) {
            if (auto const cxxname = getAnnotationArg(self.type->annotations, "cxxname"); cxxname) {
                return os << schema::valueCast<std::string_view>(*cxxname);
            }
            if (auto const cxximport = getAnnotationArg(self.type->annotations, "cxximport"); cxximport) {
                return os << schema::valueCast<std::string_view>(*cxximport);
            }

            return os << TypeNamespacePrefix{self.type} << Ident{self.type->name};
        }
    };

    struct Type {
        schema::Module const* mod = nullptr;
        schema::TypeBase const* type = nullptr;

        friend std::ostream& operator<<(std::ostream& os, Type const&& self) {
            using namespace schema;

            switch (self.type->kind) {
                case TypeKind::Pointer:
                    return os << "up::box<" << Type{self.mod, static_cast<TypeIndirect const*>(self.type)->ref} << ">";
                case TypeKind::Alias:
                    return os << Type{self.mod, static_cast<TypeIndirect const*>(self.type)->ref};
                case TypeKind::Array:
                    return os << "up::vector<" << Type{self.mod, static_cast<TypeIndirect const*>(self.type)->ref}
                              << ">";
                case TypeKind::Specialized:
                    os << Type{self.mod, static_cast<TypeSpecialized const*>(self.type)->ref} << "<";
                    {
                        bool first = true;
                        for (auto const* typeArg : static_cast<TypeSpecialized const*>(self.type)->typeArgs) {
                            if (first) {
                                first = false;
                            }
                            else {
                                os << ", ";
                            }
                            os << Type{self.mod, typeArg};
                        }
                    }
                    return os << ">";

                    break;
                default:
                    return os << QualifiedName{self.type};
            }
        }
    };

    inline std::string typeNamespace(schema::TypeBase const& type) {
        using namespace std::literals;

        auto const anno = getAnnotationArg(type.annotations, "cxxnamespace");
        if (anno) {
            valueCast<std::string>(*anno);
        }
        if (type.kind == schema::TypeKind::Attribute) {
            return "up::schema::attribute"s;
        }
        return "up::schema"s;
    }
} // namespace cxx

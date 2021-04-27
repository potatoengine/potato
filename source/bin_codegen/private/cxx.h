// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "schema.h"

#include <ostream>
#include <sstream>
#include <string_view>

namespace cxx {
    struct Ident {
        std::string_view ident;

        friend std::ostream& operator<<(std::ostream& os, Ident const&& self) { return os << self.ident; }
    };

    struct TypeNamespace {
        schema::TypeBase const& type;

        friend std::ostream& operator<<(std::ostream& os, TypeNamespace const&& self) {
            using namespace std::literals;

            if (auto const cxxns = getAnnotationArg(self.type.annotations, "cxxnamespace"); cxxns) {
                return os << schema::valueCast<std::string_view>(*cxxns);
            }
            // if (self.type->kind == schema::TypeKind::Attribute) {
            //    return os << "schema::attribute::";
            //}
            if (self.type.qualifiedName == "int"sv || self.type.qualifiedName == "float"sv ||
                self.type.qualifiedName == "bool"sv) {
                return os;
            }
            if (self.type.qualifiedName == "string"sv) {
                return os << "up";
            }
            return os << "up::schema";
        }
    };

    struct TypeNamespacePrefix {
        schema::TypeBase const& type;

        friend std::ostream& operator<<(std::ostream& os, TypeNamespacePrefix const&& self) {
            std::ostringstream buf;
            buf << TypeNamespace{self.type};
            os << buf.str();
            if (!buf.str().empty()) {
                os << "::";
            }
            return os;
        }
    };

    struct QualifiedName {
        schema::TypeBase const& type;

        friend std::ostream& operator<<(std::ostream& os, QualifiedName const&& self) {
            if (auto const cxxname = getAnnotationArg(self.type.annotations, "cxxname"); cxxname) {
                return os << schema::valueCast<std::string_view>(*cxxname);
            }
            if (auto const cxximport = getAnnotationArg(self.type.annotations, "cxximport"); cxximport) {
                return os << schema::valueCast<std::string_view>(*cxximport);
            }

            return os << TypeNamespacePrefix{self.type} << Ident{self.type.name};
        }
    };

    struct Type {
        schema::Module const& mod;
        schema::TypeBase const& type;

        friend std::ostream& operator<<(std::ostream& os, Type const&& self) {
            using namespace schema;

            switch (self.type.kind) {
                case TypeKind::Pointer:
                    return os << "up::box<" << Type{self.mod, *static_cast<TypeIndirect const&>(self.type).ref} << ">";
                case TypeKind::Alias:
                    return os << Type{self.mod, *static_cast<TypeIndirect const&>(self.type).ref};
                case TypeKind::Array:
                    return os << "up::vector<" << Type{self.mod, *static_cast<TypeIndirect const&>(self.type).ref}
                              << ">";
                case TypeKind::Specialized:
                    if (auto const cxximport = getAnnotationArg(
                            static_cast<TypeSpecialized const&>(self.type).ref->annotations,
                            "cxximport");
                        cxximport) {
                        expandPlaceholders(os, valueCast<std::string>(*cxximport), self);
                    }
                    else {
                        buildSpecialization(os, self);
                    }
                    return os;
                default:
                    return os << QualifiedName{self.type};
            }
        }

        static void expandPlaceholders(std::ostream& os, std::string_view cxxname, Type const& self) {
            auto const& type = static_cast<schema::TypeSpecialized const&>(self.type);
            auto const& aggr = static_cast<schema::TypeAggregate const&>(*type.ref);

            auto pos = cxxname.find('$');
            decltype(pos) last = 0;
            while (pos != std::string::npos) {
                for (size_t index = 0; index != aggr.typeParams.size(); ++index) {
                    auto const& typeParam = aggr.typeParams[index]->name;

                    if (std::string_view{cxxname}.substr(pos + 1).starts_with(typeParam)) {
                        os.write(cxxname.data() + last, pos - last);
                        os << QualifiedName{*type.typeArgs[index]};
                        last = pos = pos + typeParam.length() + 1;
                        break;
                    }
                }
                pos = cxxname.find('$', pos);
            }

            os << std::string_view{cxxname}.substr(last);
        }

        static void buildSpecialization(std::ostream& os, Type const& self) {
            auto const& type = static_cast<schema::TypeSpecialized const&>(self.type);

            os << Type{self.mod, *type.ref} << "<";
            for (auto const* typeArg : type.typeArgs) {
                if (typeArg != type.typeArgs.front()) {
                    os << ", ";
                }
                os << Type{self.mod, *typeArg};
            }
            os << ">";
        }
    };

    inline std::string typeNamespace(schema::TypeBase const& type) {
        std::ostringstream buf;
        buf << TypeNamespace{type};
        return buf.str();
    }
} // namespace cxx

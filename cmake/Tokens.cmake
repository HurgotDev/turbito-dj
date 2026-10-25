# Generates ui/look/Tokens.h from design/tokens.json at configure time.
set(TOKENS_JSON ${PROJECT_SOURCE_DIR}/design/tokens.json)
set(TOKENS_HEADER ${CMAKE_BINARY_DIR}/generated/ui/look/Tokens.h)

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${TOKENS_JSON})
file(READ ${TOKENS_JSON} TOKENS_JSON_TEXT)

set_property(GLOBAL PROPERTY TOKENS_BODY "")

function(tokens_add text)
    set_property(GLOBAL APPEND_STRING PROPERTY TOKENS_BODY "    ${text}\n")
endfunction()

function(tokens_color out text)
    if(NOT "${text}" MATCHES "^#[0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]$")
        message(FATAL_ERROR "tokens.json: not a #RRGGBB color: ${text}")
    endif()
    string(SUBSTRING "${text}" 1 -1 hex)
    set(${out} "0xff${hex}" PARENT_SCOPE)
endfunction()

# Emits one declaration per member of NODE (JSON text). Names are PREFIX plus
# the capitalized key ("vu" + "low" -> vuLow); an empty PREFIX keeps the key.
function(tokens_members node prefix)
    string(JSON count LENGTH "${node}")
    math(EXPR last "${count} - 1")
    foreach(i RANGE ${last})
        string(JSON key MEMBER "${node}" ${i})
        string(JSON value GET "${node}" ${key})
        string(JSON kind TYPE "${node}" ${key})

        if("${prefix}" STREQUAL "" OR "${key}" STREQUAL "labelTracking")
            set(name ${key})
        else()
            string(SUBSTRING "${key}" 0 1 head)
            string(TOUPPER "${head}" head)
            string(SUBSTRING "${key}" 1 -1 tail)
            set(name "${prefix}${head}${tail}")
        endif()

        if("${kind}" STREQUAL "OBJECT")
            tokens_members("${value}" "${name}")
        elseif("${kind}" STREQUAL "ARRAY")
            string(JSON size LENGTH "${value}")
            math(EXPR last_item "${size} - 1")
            set(colors "")
            foreach(j RANGE ${last_item})
                string(JSON item GET "${value}" ${j})
                tokens_color(color "${item}")
                list(APPEND colors ${color})
            endforeach()
            string(JOIN ", " joined ${colors})
            tokens_add("inline constexpr std::array<std::uint32_t, ${size}> ${name} { ${joined} };")
        elseif("${kind}" STREQUAL "NUMBER")
            tokens_add("inline constexpr int ${name} = ${value};")
        elseif("${value}" MATCHES "^#")
            tokens_color(color "${value}")
            tokens_add("inline constexpr std::uint32_t ${name} = ${color};")
        elseif("${value}" MATCHES "em$")
            string(REGEX REPLACE "em$" "" number "${value}")
            tokens_add("inline constexpr float ${name} = ${number}f;")
        else()
            tokens_add("inline constexpr const char* ${name} = \"${value}\";")
        endif()
    endforeach()
endfunction()

string(JSON window_w GET "${TOKENS_JSON_TEXT}" window reference 0)
string(JSON window_h GET "${TOKENS_JSON_TEXT}" window reference 1)
tokens_add("inline constexpr Size windowReference { ${window_w}, ${window_h} };")
string(JSON window_w GET "${TOKENS_JSON_TEXT}" window minimum 0)
string(JSON window_h GET "${TOKENS_JSON_TEXT}" window minimum 1)
tokens_add("inline constexpr Size windowMinimum { ${window_w}, ${window_h} };")

string(JSON node GET "${TOKENS_JSON_TEXT}" color)
tokens_members("${node}" "")
string(JSON node GET "${TOKENS_JSON_TEXT}" font)
tokens_members("${node}" "font")
string(JSON node GET "${TOKENS_JSON_TEXT}" radius)
tokens_members("${node}" "radius")
string(JSON node GET "${TOKENS_JSON_TEXT}" spacing)
tokens_members("${node}" "spacing")
string(JSON node GET "${TOKENS_JSON_TEXT}" layout)
tokens_members("${node}" "layout")

get_property(body GLOBAL PROPERTY TOKENS_BODY)
set(header "// Generated from design/tokens.json by cmake/Tokens.cmake. Do not edit.
#pragma once

#include <array>
#include <cstdint>

namespace tokens
{
    struct Size
    {
        int width;
        int height;
    };

")

file(CONFIGURE OUTPUT ${TOKENS_HEADER} CONTENT "${header}${body}}\n")

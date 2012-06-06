// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

// This file was generated using name_tool.py

#include "common_headers.h"
#include "names_builtin.h"

namespace circa {

const char* builtin_name_to_string(int name)
{
    if (name >= name_LastBuiltinName)
        return NULL;

    switch (name) {
    case name_None: return "None";
    case name_File: return "File";
    case name_Newline: return "Newline";
    case name_Out: return "Out";
    case name_Unknown: return "Unknown";
    case name_Repeat: return "Repeat";
    case name_Success: return "Success";
    case name_Failure: return "Failure";
    case name_FileNotFound: return "FileNotFound";
    case name_NotEnoughInputs: return "NotEnoughInputs";
    case name_TooManyInputs: return "TooManyInputs";
    case name_ExtraOutputNotFound: return "ExtraOutputNotFound";
    case name_Default: return "Default";
    case name_ByDemand: return "ByDemand";
    case name_Unevaluated: return "Unevaluated";
    case name_InProgress: return "InProgress";
    case name_Lazy: return "Lazy";
    case name_Consumed: return "Consumed";
    case name_Return: return "Return";
    case name_Continue: return "Continue";
    case name_Break: return "Break";
    case name_Discard: return "Discard";
    case name_InfixOperator: return "InfixOperator";
    case name_FunctionName: return "FunctionName";
    case name_TypeName: return "TypeName";
    case name_TermName: return "TermName";
    case name_Keyword: return "Keyword";
    case name_Whitespace: return "Whitespace";
    case name_UnknownIdentifier: return "UnknownIdentifier";
    case tok_Identifier: return "tok_Identifier";
    case tok_Name: return "tok_Name";
    case tok_Integer: return "tok_Integer";
    case tok_HexInteger: return "tok_HexInteger";
    case tok_Float: return "tok_Float";
    case tok_String: return "tok_String";
    case tok_Color: return "tok_Color";
    case tok_Bool: return "tok_Bool";
    case tok_LParen: return "tok_LParen";
    case tok_RParen: return "tok_RParen";
    case tok_LBrace: return "tok_LBrace";
    case tok_RBrace: return "tok_RBrace";
    case tok_LBracket: return "tok_LBracket";
    case tok_RBracket: return "tok_RBracket";
    case tok_Comma: return "tok_Comma";
    case tok_At: return "tok_At";
    case tok_AtDot: return "tok_AtDot";
    case tok_Dot: return "tok_Dot";
    case tok_Star: return "tok_Star";
    case tok_Question: return "tok_Question";
    case tok_Slash: return "tok_Slash";
    case tok_DoubleSlash: return "tok_DoubleSlash";
    case tok_Plus: return "tok_Plus";
    case tok_Minus: return "tok_Minus";
    case tok_LThan: return "tok_LThan";
    case tok_LThanEq: return "tok_LThanEq";
    case tok_GThan: return "tok_GThan";
    case tok_GThanEq: return "tok_GThanEq";
    case tok_Percent: return "tok_Percent";
    case tok_Colon: return "tok_Colon";
    case tok_DoubleColon: return "tok_DoubleColon";
    case tok_DoubleEquals: return "tok_DoubleEquals";
    case tok_NotEquals: return "tok_NotEquals";
    case tok_Equals: return "tok_Equals";
    case tok_PlusEquals: return "tok_PlusEquals";
    case tok_MinusEquals: return "tok_MinusEquals";
    case tok_StarEquals: return "tok_StarEquals";
    case tok_SlashEquals: return "tok_SlashEquals";
    case tok_ColonEquals: return "tok_ColonEquals";
    case tok_RightArrow: return "tok_RightArrow";
    case tok_LeftArrow: return "tok_LeftArrow";
    case tok_Ampersand: return "tok_Ampersand";
    case tok_DoubleAmpersand: return "tok_DoubleAmpersand";
    case tok_DoubleVerticalBar: return "tok_DoubleVerticalBar";
    case tok_Semicolon: return "tok_Semicolon";
    case tok_TwoDots: return "tok_TwoDots";
    case tok_Ellipsis: return "tok_Ellipsis";
    case tok_TripleLThan: return "tok_TripleLThan";
    case tok_TripleGThan: return "tok_TripleGThan";
    case tok_Pound: return "tok_Pound";
    case tok_Def: return "tok_Def";
    case tok_Type: return "tok_Type";
    case tok_Begin: return "tok_Begin";
    case tok_Do: return "tok_Do";
    case tok_End: return "tok_End";
    case tok_If: return "tok_If";
    case tok_Else: return "tok_Else";
    case tok_Elif: return "tok_Elif";
    case tok_For: return "tok_For";
    case tok_State: return "tok_State";
    case tok_Return: return "tok_Return";
    case tok_In: return "tok_In";
    case tok_True: return "tok_True";
    case tok_False: return "tok_False";
    case tok_DoOnce: return "tok_DoOnce";
    case tok_Namespace: return "tok_Namespace";
    case tok_Include: return "tok_Include";
    case tok_Import: return "tok_Import";
    case tok_And: return "tok_And";
    case tok_OR: return "tok_OR";
    case tok_Not: return "tok_Not";
    case tok_Discard: return "tok_Discard";
    case tok_Null: return "tok_Null";
    case tok_Break: return "tok_Break";
    case tok_Continue: return "tok_Continue";
    case tok_Switch: return "tok_Switch";
    case tok_Case: return "tok_Case";
    case tok_While: return "tok_While";
    case tok_Whitespace: return "tok_Whitespace";
    case tok_Newline: return "tok_Newline";
    case tok_Comment: return "tok_Comment";
    case tok_Eof: return "tok_Eof";
    case tok_Unrecognized: return "tok_Unrecognized";
    case name_LastBuiltinName: return "LastBuiltinName";
    default: return NULL;
    }
}

} // namespace circa

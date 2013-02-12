// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

// This file was generated using name_tool.py

#pragma once

namespace circa {

const int sym_None = 0;
const int sym_Invalid = 1;
const int sym_File = 2;
const int sym_Newline = 3;
const int sym_Out = 4;
const int sym_Unknown = 5;
const int sym_Repeat = 6;
const int sym_Success = 7;
const int sym_Failure = 8;
const int sym_Yes = 9;
const int sym_No = 10;
const int sym_Maybe = 11;
const int sym_EvaluationEmpty = 12;
const int sym_HasEffects = 13;
const int sym_Origin = 14;
const int sym_HasControlFlow = 15;
const int sym_Wildcard = 16;
const int sym_RecursiveWildcard = 17;
const int sym_Function = 18;
const int sym_FileNotFound = 19;
const int sym_NotEnoughInputs = 20;
const int sym_TooManyInputs = 21;
const int sym_ExtraOutputNotFound = 22;
const int sym_Default = 23;
const int sym_ByDemand = 24;
const int sym_Unevaluated = 25;
const int sym_InProgress = 26;
const int sym_Lazy = 27;
const int sym_Consumed = 28;
const int sym_Uncaptured = 29;
const int sym_Return = 30;
const int sym_Continue = 31;
const int sym_Break = 32;
const int sym_Discard = 33;
const int sym_Control = 34;
const int sym_ExitLevelFunction = 35;
const int sym_ExitLevelLoop = 36;
const int sym_HighestExitLevel = 37;
const int sym_ExtraReturn = 38;
const int sym_Name = 39;
const int sym_Primary = 40;
const int sym_Anonymous = 41;
const int sym_State = 42;
const int sym_InfixOperator = 43;
const int sym_FunctionName = 44;
const int sym_TypeName = 45;
const int sym_TermName = 46;
const int sym_Keyword = 47;
const int sym_Whitespace = 48;
const int sym_UnknownIdentifier = 49;
const int sym_LookupAny = 50;
const int sym_LookupType = 51;
const int sym_LookupFunction = 52;
const int sym_LookupModule = 53;
const int sym_Untyped = 54;
const int sym_UniformListType = 55;
const int sym_AnonStructType = 56;
const int sym_StructType = 57;
const int sym_NativePatch = 58;
const int sym_PatchBlock = 59;
const int sym_Bootstrapping = 60;
const int sym_Done = 61;
const int sym_StorageTypeNull = 62;
const int sym_StorageTypeInt = 63;
const int sym_StorageTypeFloat = 64;
const int sym_StorageTypeBool = 65;
const int sym_StorageTypeString = 66;
const int sym_StorageTypeList = 67;
const int sym_StorageTypeOpaquePointer = 68;
const int sym_StorageTypeTerm = 69;
const int sym_StorageTypeType = 70;
const int sym_StorageTypeHandle = 71;
const int sym_StorageTypeHashtable = 72;
const int sym_StorageTypeObject = 73;
const int tok_Identifier = 74;
const int tok_ColonString = 75;
const int tok_Integer = 76;
const int tok_HexInteger = 77;
const int tok_Float = 78;
const int tok_String = 79;
const int tok_Color = 80;
const int tok_Bool = 81;
const int tok_LParen = 82;
const int tok_RParen = 83;
const int tok_LBrace = 84;
const int tok_RBrace = 85;
const int tok_LBracket = 86;
const int tok_RBracket = 87;
const int tok_Comma = 88;
const int tok_At = 89;
const int tok_Dot = 90;
const int tok_DotAt = 91;
const int tok_Star = 92;
const int tok_DoubleStar = 93;
const int tok_Question = 94;
const int tok_Slash = 95;
const int tok_DoubleSlash = 96;
const int tok_Plus = 97;
const int tok_Minus = 98;
const int tok_LThan = 99;
const int tok_LThanEq = 100;
const int tok_GThan = 101;
const int tok_GThanEq = 102;
const int tok_Percent = 103;
const int tok_Colon = 104;
const int tok_DoubleColon = 105;
const int tok_DoubleEquals = 106;
const int tok_NotEquals = 107;
const int tok_Equals = 108;
const int tok_PlusEquals = 109;
const int tok_MinusEquals = 110;
const int tok_StarEquals = 111;
const int tok_SlashEquals = 112;
const int tok_ColonEquals = 113;
const int tok_RightArrow = 114;
const int tok_LeftArrow = 115;
const int tok_Ampersand = 116;
const int tok_DoubleAmpersand = 117;
const int tok_DoubleVerticalBar = 118;
const int tok_Semicolon = 119;
const int tok_TwoDots = 120;
const int tok_Ellipsis = 121;
const int tok_TripleLThan = 122;
const int tok_TripleGThan = 123;
const int tok_Pound = 124;
const int tok_Def = 125;
const int tok_Type = 126;
const int tok_UnusedName1 = 127;
const int tok_UnusedName2 = 128;
const int tok_UnusedName3 = 129;
const int tok_If = 130;
const int tok_Else = 131;
const int tok_Elif = 132;
const int tok_For = 133;
const int tok_State = 134;
const int tok_Return = 135;
const int tok_In = 136;
const int tok_True = 137;
const int tok_False = 138;
const int tok_Namespace = 139;
const int tok_Include = 140;
const int tok_And = 141;
const int tok_Or = 142;
const int tok_Not = 143;
const int tok_Discard = 144;
const int tok_Null = 145;
const int tok_Break = 146;
const int tok_Continue = 147;
const int tok_Switch = 148;
const int tok_Case = 149;
const int tok_While = 150;
const int tok_Require = 151;
const int tok_Package = 152;
const int tok_Section = 153;
const int tok_Whitespace = 154;
const int tok_Newline = 155;
const int tok_Comment = 156;
const int tok_Eof = 157;
const int tok_Unrecognized = 158;
const int op_NoOp = 159;
const int op_Pause = 160;
const int op_SetNull = 161;
const int op_InlineCopy = 162;
const int op_CallBlock = 163;
const int op_DynamicCall = 164;
const int op_ClosureCall = 165;
const int op_FireNative = 166;
const int op_CaseBlock = 167;
const int op_ForLoop = 168;
const int op_ExitPoint = 169;
const int op_Return = 170;
const int op_Continue = 171;
const int op_Break = 172;
const int op_Discard = 173;
const int op_FinishFrame = 174;
const int op_FinishLoop = 175;
const int op_ErrorNotEnoughInputs = 176;
const int op_ErrorTooManyInputs = 177;
const int sym_LoopProduceOutput = 178;
const int sym_FlatOutputs = 179;
const int sym_OutputsToList = 180;
const int sym_Multiple = 181;
const int sym_Cast = 182;
const int sym_DynamicMethodOutput = 183;
const int sym_FirstStatIndex = 184;
const int stat_TermsCreated = 185;
const int stat_TermPropAdded = 186;
const int stat_TermPropAccess = 187;
const int stat_InternedNameLookup = 188;
const int stat_InternedNameCreate = 189;
const int stat_Copy_PushedInputNewFrame = 190;
const int stat_Copy_PushedInputMultiNewFrame = 191;
const int stat_Copy_PushFrameWithInputs = 192;
const int stat_Copy_ListDuplicate = 193;
const int stat_Copy_LoopCopyRebound = 194;
const int stat_Cast_ListCastElement = 195;
const int stat_Cast_PushFrameWithInputs = 196;
const int stat_Cast_FinishFrame = 197;
const int stat_Touch_ListCast = 198;
const int stat_ValueCreates = 199;
const int stat_ValueCopies = 200;
const int stat_ValueCast = 201;
const int stat_ValueCastDispatched = 202;
const int stat_ValueTouch = 203;
const int stat_ListsCreated = 204;
const int stat_ListsGrown = 205;
const int stat_ListSoftCopy = 206;
const int stat_ListHardCopy = 207;
const int stat_DictHardCopy = 208;
const int stat_StringCreate = 209;
const int stat_StringDuplicate = 210;
const int stat_StringResizeInPlace = 211;
const int stat_StringResizeCreate = 212;
const int stat_StringSoftCopy = 213;
const int stat_StringToStd = 214;
const int stat_StepInterpreter = 215;
const int stat_InterpreterCastOutputFromFinishedFrame = 216;
const int stat_BlockNameLookups = 217;
const int stat_PushFrame = 218;
const int stat_LoopFinishIteration = 219;
const int stat_LoopWriteOutput = 220;
const int stat_WriteTermBytecode = 221;
const int stat_DynamicCall = 222;
const int stat_FinishDynamicCall = 223;
const int stat_DynamicMethodCall = 224;
const int stat_SetIndex = 225;
const int stat_SetField = 226;
const int sym_LastStatIndex = 227;
const int sym_LastBuiltinName = 228;

const char* builtin_symbol_to_string(int name);
int builtin_symbol_from_string(const char* str);

} // namespace circa

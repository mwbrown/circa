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
const int sym_Index = 12;
const int sym_Last = 13;
const int sym_EvaluationEmpty = 14;
const int sym_HasEffects = 15;
const int sym_HasControlFlow = 16;
const int sym_DirtyStateType = 17;
const int sym_Filename = 18;
const int sym_Builtins = 19;
const int sym_Wildcard = 20;
const int sym_RecursiveWildcard = 21;
const int sym_Function = 22;
const int sym_FileNotFound = 23;
const int sym_NotEnoughInputs = 24;
const int sym_TooManyInputs = 25;
const int sym_ExtraOutputNotFound = 26;
const int sym_Default = 27;
const int sym_ByDemand = 28;
const int sym_Unevaluated = 29;
const int sym_InProgress = 30;
const int sym_Lazy = 31;
const int sym_Consumed = 32;
const int sym_Uncaptured = 33;
const int sym_Return = 34;
const int sym_Continue = 35;
const int sym_Break = 36;
const int sym_Discard = 37;
const int sym_Control = 38;
const int sym_ExitLevelFunction = 39;
const int sym_ExitLevelLoop = 40;
const int sym_HighestExitLevel = 41;
const int sym_ExtraReturn = 42;
const int sym_Name = 43;
const int sym_Primary = 44;
const int sym_Anonymous = 45;
const int sym_State = 46;
const int sym_StackReady = 47;
const int sym_StackRunning = 48;
const int sym_StackFinished = 49;
const int sym_InfixOperator = 50;
const int sym_FunctionName = 51;
const int sym_TypeName = 52;
const int sym_TermName = 53;
const int sym_Keyword = 54;
const int sym_Whitespace = 55;
const int sym_UnknownIdentifier = 56;
const int sym_LookupAny = 57;
const int sym_LookupType = 58;
const int sym_LookupFunction = 59;
const int sym_LookupModule = 60;
const int sym_Untyped = 61;
const int sym_UniformListType = 62;
const int sym_AnonStructType = 63;
const int sym_StructType = 64;
const int sym_NativePatch = 65;
const int sym_PatchBlock = 66;
const int sym_Bootstrapping = 67;
const int sym_Done = 68;
const int sym_StorageTypeNull = 69;
const int sym_StorageTypeInt = 70;
const int sym_StorageTypeFloat = 71;
const int sym_StorageTypeBool = 72;
const int sym_StorageTypeStack = 73;
const int sym_StorageTypeString = 74;
const int sym_StorageTypeList = 75;
const int sym_StorageTypeOpaquePointer = 76;
const int sym_StorageTypeTerm = 77;
const int sym_StorageTypeType = 78;
const int sym_StorageTypeHandle = 79;
const int sym_StorageTypeHashtable = 80;
const int sym_StorageTypeObject = 81;
const int tok_Identifier = 82;
const int tok_ColonString = 83;
const int tok_Integer = 84;
const int tok_HexInteger = 85;
const int tok_Float = 86;
const int tok_String = 87;
const int tok_Color = 88;
const int tok_Bool = 89;
const int tok_LParen = 90;
const int tok_RParen = 91;
const int tok_LBrace = 92;
const int tok_RBrace = 93;
const int tok_LBracket = 94;
const int tok_RBracket = 95;
const int tok_Comma = 96;
const int tok_At = 97;
const int tok_Dot = 98;
const int tok_DotAt = 99;
const int tok_Star = 100;
const int tok_DoubleStar = 101;
const int tok_Question = 102;
const int tok_Slash = 103;
const int tok_DoubleSlash = 104;
const int tok_Plus = 105;
const int tok_Minus = 106;
const int tok_LThan = 107;
const int tok_LThanEq = 108;
const int tok_GThan = 109;
const int tok_GThanEq = 110;
const int tok_Percent = 111;
const int tok_Colon = 112;
const int tok_DoubleColon = 113;
const int tok_DoubleEquals = 114;
const int tok_NotEquals = 115;
const int tok_Equals = 116;
const int tok_PlusEquals = 117;
const int tok_MinusEquals = 118;
const int tok_StarEquals = 119;
const int tok_SlashEquals = 120;
const int tok_ColonEquals = 121;
const int tok_RightArrow = 122;
const int tok_LeftArrow = 123;
const int tok_Ampersand = 124;
const int tok_DoubleAmpersand = 125;
const int tok_DoubleVerticalBar = 126;
const int tok_Semicolon = 127;
const int tok_TwoDots = 128;
const int tok_Ellipsis = 129;
const int tok_TripleLThan = 130;
const int tok_TripleGThan = 131;
const int tok_Pound = 132;
const int tok_Def = 133;
const int tok_Type = 134;
const int tok_UnusedName1 = 135;
const int tok_UnusedName2 = 136;
const int tok_UnusedName3 = 137;
const int tok_If = 138;
const int tok_Else = 139;
const int tok_Elif = 140;
const int tok_For = 141;
const int tok_State = 142;
const int tok_Return = 143;
const int tok_In = 144;
const int tok_True = 145;
const int tok_False = 146;
const int tok_Namespace = 147;
const int tok_Include = 148;
const int tok_And = 149;
const int tok_Or = 150;
const int tok_Not = 151;
const int tok_Discard = 152;
const int tok_Null = 153;
const int tok_Break = 154;
const int tok_Continue = 155;
const int tok_Switch = 156;
const int tok_Case = 157;
const int tok_While = 158;
const int tok_Require = 159;
const int tok_Package = 160;
const int tok_Section = 161;
const int tok_Whitespace = 162;
const int tok_Newline = 163;
const int tok_Comment = 164;
const int tok_Eof = 165;
const int tok_Unrecognized = 166;
const int op_NoOp = 167;
const int op_Pause = 168;
const int op_SetNull = 169;
const int op_InlineCopy = 170;
const int op_CallBlock = 171;
const int op_DynamicMethodCall = 172;
const int op_ClosureCall = 173;
const int op_ClosureApply = 174;
const int op_FireNative = 175;
const int op_CaseBlock = 176;
const int op_ForLoop = 177;
const int op_ExitPoint = 178;
const int op_Return = 179;
const int op_Continue = 180;
const int op_Break = 181;
const int op_Discard = 182;
const int op_FinishFrame = 183;
const int op_FinishLoop = 184;
const int op_ErrorNotEnoughInputs = 185;
const int op_ErrorTooManyInputs = 186;
const int sym_LoopProduceOutput = 187;
const int sym_FlatOutputs = 188;
const int sym_OutputsToList = 189;
const int sym_Multiple = 190;
const int sym_Cast = 191;
const int sym_DynamicMethodOutput = 192;
const int sym_FirstStatIndex = 193;
const int stat_TermsCreated = 194;
const int stat_TermPropAdded = 195;
const int stat_TermPropAccess = 196;
const int stat_InternedNameLookup = 197;
const int stat_InternedNameCreate = 198;
const int stat_Copy_PushedInputNewFrame = 199;
const int stat_Copy_PushedInputMultiNewFrame = 200;
const int stat_Copy_PushFrameWithInputs = 201;
const int stat_Copy_ListDuplicate = 202;
const int stat_Copy_LoopCopyRebound = 203;
const int stat_Cast_ListCastElement = 204;
const int stat_Cast_PushFrameWithInputs = 205;
const int stat_Cast_FinishFrame = 206;
const int stat_Touch_ListCast = 207;
const int stat_ValueCreates = 208;
const int stat_ValueCopies = 209;
const int stat_ValueCast = 210;
const int stat_ValueCastDispatched = 211;
const int stat_ValueTouch = 212;
const int stat_ListsCreated = 213;
const int stat_ListsGrown = 214;
const int stat_ListSoftCopy = 215;
const int stat_ListHardCopy = 216;
const int stat_DictHardCopy = 217;
const int stat_StringCreate = 218;
const int stat_StringDuplicate = 219;
const int stat_StringResizeInPlace = 220;
const int stat_StringResizeCreate = 221;
const int stat_StringSoftCopy = 222;
const int stat_StringToStd = 223;
const int stat_StepInterpreter = 224;
const int stat_InterpreterCastOutputFromFinishedFrame = 225;
const int stat_BlockNameLookups = 226;
const int stat_PushFrame = 227;
const int stat_LoopFinishIteration = 228;
const int stat_LoopWriteOutput = 229;
const int stat_WriteTermBytecode = 230;
const int stat_DynamicCall = 231;
const int stat_FinishDynamicCall = 232;
const int stat_DynamicMethodCall = 233;
const int stat_SetIndex = 234;
const int stat_SetField = 235;
const int sym_LastStatIndex = 236;
const int sym_LastBuiltinName = 237;

const char* builtin_symbol_to_string(int name);
int builtin_symbol_from_string(const char* str);

} // namespace circa

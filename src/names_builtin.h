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
const int sym_DirtyStateType = 16;
const int sym_Wildcard = 17;
const int sym_RecursiveWildcard = 18;
const int sym_Function = 19;
const int sym_FileNotFound = 20;
const int sym_NotEnoughInputs = 21;
const int sym_TooManyInputs = 22;
const int sym_ExtraOutputNotFound = 23;
const int sym_Default = 24;
const int sym_ByDemand = 25;
const int sym_Unevaluated = 26;
const int sym_InProgress = 27;
const int sym_Lazy = 28;
const int sym_Consumed = 29;
const int sym_Uncaptured = 30;
const int sym_Return = 31;
const int sym_Continue = 32;
const int sym_Break = 33;
const int sym_Discard = 34;
const int sym_Control = 35;
const int sym_ExitLevelFunction = 36;
const int sym_ExitLevelLoop = 37;
const int sym_HighestExitLevel = 38;
const int sym_ExtraReturn = 39;
const int sym_Name = 40;
const int sym_Primary = 41;
const int sym_Anonymous = 42;
const int sym_State = 43;
const int sym_StackReady = 44;
const int sym_StackRunning = 45;
const int sym_StackFinished = 46;
const int sym_InfixOperator = 47;
const int sym_FunctionName = 48;
const int sym_TypeName = 49;
const int sym_TermName = 50;
const int sym_Keyword = 51;
const int sym_Whitespace = 52;
const int sym_UnknownIdentifier = 53;
const int sym_LookupAny = 54;
const int sym_LookupType = 55;
const int sym_LookupFunction = 56;
const int sym_LookupModule = 57;
const int sym_Untyped = 58;
const int sym_UniformListType = 59;
const int sym_AnonStructType = 60;
const int sym_StructType = 61;
const int sym_NativePatch = 62;
const int sym_PatchBlock = 63;
const int sym_Bootstrapping = 64;
const int sym_Done = 65;
const int sym_StorageTypeNull = 66;
const int sym_StorageTypeInt = 67;
const int sym_StorageTypeFloat = 68;
const int sym_StorageTypeBool = 69;
const int sym_StorageTypeStack = 70;
const int sym_StorageTypeString = 71;
const int sym_StorageTypeList = 72;
const int sym_StorageTypeOpaquePointer = 73;
const int sym_StorageTypeTerm = 74;
const int sym_StorageTypeType = 75;
const int sym_StorageTypeHandle = 76;
const int sym_StorageTypeHashtable = 77;
const int sym_StorageTypeObject = 78;
const int tok_Identifier = 79;
const int tok_ColonString = 80;
const int tok_Integer = 81;
const int tok_HexInteger = 82;
const int tok_Float = 83;
const int tok_String = 84;
const int tok_Color = 85;
const int tok_Bool = 86;
const int tok_LParen = 87;
const int tok_RParen = 88;
const int tok_LBrace = 89;
const int tok_RBrace = 90;
const int tok_LBracket = 91;
const int tok_RBracket = 92;
const int tok_Comma = 93;
const int tok_At = 94;
const int tok_Dot = 95;
const int tok_DotAt = 96;
const int tok_Star = 97;
const int tok_DoubleStar = 98;
const int tok_Question = 99;
const int tok_Slash = 100;
const int tok_DoubleSlash = 101;
const int tok_Plus = 102;
const int tok_Minus = 103;
const int tok_LThan = 104;
const int tok_LThanEq = 105;
const int tok_GThan = 106;
const int tok_GThanEq = 107;
const int tok_Percent = 108;
const int tok_Colon = 109;
const int tok_DoubleColon = 110;
const int tok_DoubleEquals = 111;
const int tok_NotEquals = 112;
const int tok_Equals = 113;
const int tok_PlusEquals = 114;
const int tok_MinusEquals = 115;
const int tok_StarEquals = 116;
const int tok_SlashEquals = 117;
const int tok_ColonEquals = 118;
const int tok_RightArrow = 119;
const int tok_LeftArrow = 120;
const int tok_Ampersand = 121;
const int tok_DoubleAmpersand = 122;
const int tok_DoubleVerticalBar = 123;
const int tok_Semicolon = 124;
const int tok_TwoDots = 125;
const int tok_Ellipsis = 126;
const int tok_TripleLThan = 127;
const int tok_TripleGThan = 128;
const int tok_Pound = 129;
const int tok_Def = 130;
const int tok_Type = 131;
const int tok_UnusedName1 = 132;
const int tok_UnusedName2 = 133;
const int tok_UnusedName3 = 134;
const int tok_If = 135;
const int tok_Else = 136;
const int tok_Elif = 137;
const int tok_For = 138;
const int tok_State = 139;
const int tok_Return = 140;
const int tok_In = 141;
const int tok_True = 142;
const int tok_False = 143;
const int tok_Namespace = 144;
const int tok_Include = 145;
const int tok_And = 146;
const int tok_Or = 147;
const int tok_Not = 148;
const int tok_Discard = 149;
const int tok_Null = 150;
const int tok_Break = 151;
const int tok_Continue = 152;
const int tok_Switch = 153;
const int tok_Case = 154;
const int tok_While = 155;
const int tok_Require = 156;
const int tok_Package = 157;
const int tok_Section = 158;
const int tok_Whitespace = 159;
const int tok_Newline = 160;
const int tok_Comment = 161;
const int tok_Eof = 162;
const int tok_Unrecognized = 163;
const int op_NoOp = 164;
const int op_Pause = 165;
const int op_SetNull = 166;
const int op_InlineCopy = 167;
const int op_CallBlock = 168;
const int op_DynamicCall = 169;
const int op_DynamicMethodCall = 170;
const int op_ClosureCall = 171;
const int op_ClosureApply = 172;
const int op_FireNative = 173;
const int op_CaseBlock = 174;
const int op_ForLoop = 175;
const int op_ExitPoint = 176;
const int op_Return = 177;
const int op_Continue = 178;
const int op_Break = 179;
const int op_Discard = 180;
const int op_FinishFrame = 181;
const int op_FinishLoop = 182;
const int op_ErrorNotEnoughInputs = 183;
const int op_ErrorTooManyInputs = 184;
const int sym_LoopProduceOutput = 185;
const int sym_FlatOutputs = 186;
const int sym_OutputsToList = 187;
const int sym_Multiple = 188;
const int sym_Cast = 189;
const int sym_DynamicMethodOutput = 190;
const int sym_FirstStatIndex = 191;
const int stat_TermsCreated = 192;
const int stat_TermPropAdded = 193;
const int stat_TermPropAccess = 194;
const int stat_InternedNameLookup = 195;
const int stat_InternedNameCreate = 196;
const int stat_Copy_PushedInputNewFrame = 197;
const int stat_Copy_PushedInputMultiNewFrame = 198;
const int stat_Copy_PushFrameWithInputs = 199;
const int stat_Copy_ListDuplicate = 200;
const int stat_Copy_LoopCopyRebound = 201;
const int stat_Cast_ListCastElement = 202;
const int stat_Cast_PushFrameWithInputs = 203;
const int stat_Cast_FinishFrame = 204;
const int stat_Touch_ListCast = 205;
const int stat_ValueCreates = 206;
const int stat_ValueCopies = 207;
const int stat_ValueCast = 208;
const int stat_ValueCastDispatched = 209;
const int stat_ValueTouch = 210;
const int stat_ListsCreated = 211;
const int stat_ListsGrown = 212;
const int stat_ListSoftCopy = 213;
const int stat_ListHardCopy = 214;
const int stat_DictHardCopy = 215;
const int stat_StringCreate = 216;
const int stat_StringDuplicate = 217;
const int stat_StringResizeInPlace = 218;
const int stat_StringResizeCreate = 219;
const int stat_StringSoftCopy = 220;
const int stat_StringToStd = 221;
const int stat_StepInterpreter = 222;
const int stat_InterpreterCastOutputFromFinishedFrame = 223;
const int stat_BlockNameLookups = 224;
const int stat_PushFrame = 225;
const int stat_LoopFinishIteration = 226;
const int stat_LoopWriteOutput = 227;
const int stat_WriteTermBytecode = 228;
const int stat_DynamicCall = 229;
const int stat_FinishDynamicCall = 230;
const int stat_DynamicMethodCall = 231;
const int stat_SetIndex = 232;
const int stat_SetField = 233;
const int sym_LastStatIndex = 234;
const int sym_LastBuiltinName = 235;

const char* builtin_symbol_to_string(int name);
int builtin_symbol_from_string(const char* str);

} // namespace circa

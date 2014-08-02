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
const int sym_HasState = 17;
const int sym_HasDynamicDispatch = 18;
const int sym_DirtyStateType = 19;
const int sym_Filename = 20;
const int sym_Builtins = 21;
const int sym_ModuleName = 22;
const int sym_StaticErrors = 23;
const int sym_AccumulatingOutput = 24;
const int sym_Comment = 25;
const int sym_Constructor = 26;
const int sym_Error = 27;
const int sym_ExplicitState = 28;
const int sym_ExplicitType = 29;
const int sym_Field = 30;
const int sym_FieldAccessor = 31;
const int sym_Final = 32;
const int sym_Hidden = 33;
const int sym_HiddenInput = 34;
const int sym_Implicit = 35;
const int sym_IgnoreError = 36;
const int sym_LocalStateResult = 37;
const int sym_Meta = 38;
const int sym_Message = 39;
const int sym_MethodName = 40;
const int sym_ModifyList = 41;
const int sym_Multiple = 42;
const int sym_Mutability = 43;
const int sym_Optional = 44;
const int sym_OriginalText = 45;
const int sym_OverloadedFunc = 46;
const int sym_Ref = 47;
const int sym_Rebind = 48;
const int sym_RebindsInput = 49;
const int sym_Setter = 50;
const int sym_State = 51;
const int sym_Step = 52;
const int sym_Statement = 53;
const int sym_Output = 54;
const int sym_PreferSpecialize = 55;
const int sym_Error_UnknownType = 56;
const int sym_Syntax_AnonFunction = 57;
const int sym_Syntax_BlockStyle = 58;
const int sym_Syntax_Brackets = 59;
const int sym_Syntax_ColorFormat = 60;
const int sym_Syntax_DeclarationStyle = 61;
const int sym_Syntax_ExplicitType = 62;
const int sym_Syntax_FunctionName = 63;
const int sym_Syntax_IdentifierRebind = 64;
const int sym_Syntax_ImplicitName = 65;
const int sym_Syntax_Import = 66;
const int sym_Syntax_IntegerFormat = 67;
const int sym_Syntax_LineEnding = 68;
const int sym_Syntax_LiteralList = 69;
const int sym_Syntax_MethodDecl = 70;
const int sym_Syntax_Multiline = 71;
const int sym_Syntax_NameBinding = 72;
const int sym_Syntax_NoBrackets = 73;
const int sym_Syntax_NoParens = 74;
const int sym_Syntax_Operator = 75;
const int sym_Syntax_OriginalFormat = 76;
const int sym_Syntax_Parens = 77;
const int sym_Syntax_PreWs = 78;
const int sym_Syntax_PreDotWs = 79;
const int sym_Syntax_PreOperatorWs = 80;
const int sym_Syntax_PreEndWs = 81;
const int sym_Syntax_PreEqualsSpace = 82;
const int sym_Syntax_PreLBracketWs = 83;
const int sym_Syntax_PreRBracketWs = 84;
const int sym_Syntax_PostEqualsSpace = 85;
const int sym_Syntax_PostFunctionWs = 86;
const int sym_Syntax_PostKeywordWs = 87;
const int sym_Syntax_PostLBracketWs = 88;
const int sym_Syntax_PostHeadingWs = 89;
const int sym_Syntax_PostNameWs = 90;
const int sym_Syntax_PostWs = 91;
const int sym_Syntax_PostOperatorWs = 92;
const int sym_Syntax_Properties = 93;
const int sym_Syntax_QuoteType = 94;
const int sym_Syntax_RebindSymbol = 95;
const int sym_Syntax_RebindOperator = 96;
const int sym_Syntax_RebindingInfix = 97;
const int sym_Syntax_ReturnStatement = 98;
const int sym_Syntax_Require = 99;
const int sym_Syntax_StateKeyword = 100;
const int sym_Syntax_TypeMagicSymbol = 101;
const int sym_Syntax_WhitespaceBeforeEnd = 102;
const int sym_Syntax_WhitespacePreColon = 103;
const int sym_Syntax_WhitespacePostColon = 104;
const int sym_Wildcard = 105;
const int sym_RecursiveWildcard = 106;
const int sym_Function = 107;
const int sym_TypeRelease = 108;
const int sym_FileNotFound = 109;
const int sym_NotEnoughInputs = 110;
const int sym_TooManyInputs = 111;
const int sym_ExtraOutputNotFound = 112;
const int sym_Default = 113;
const int sym_ByDemand = 114;
const int sym_Unevaluated = 115;
const int sym_InProgress = 116;
const int sym_Lazy = 117;
const int sym_Consumed = 118;
const int sym_Uncaptured = 119;
const int sym_Return = 120;
const int sym_Continue = 121;
const int sym_Break = 122;
const int sym_Discard = 123;
const int sym_Control = 124;
const int sym_ExitLevelFunction = 125;
const int sym_ExitLevelLoop = 126;
const int sym_HighestExitLevel = 127;
const int sym_ExtraReturn = 128;
const int sym_Name = 129;
const int sym_Primary = 130;
const int sym_Anonymous = 131;
const int sym_Entropy = 132;
const int sym_OnDemand = 133;
const int sym__hacks = 134;
const int sym_no_effect = 135;
const int sym_no_save_state = 136;
const int sym_effect = 137;
const int sym_set_value = 138;
const int sym_watch = 139;
const int sym_Copy = 140;
const int sym_Move = 141;
const int sym_Unobservable = 142;
const int sym_TermCounter = 143;
const int sym_Watch = 144;
const int sym_StackReady = 145;
const int sym_StackRunning = 146;
const int sym_StackFinished = 147;
const int sym_InfixOperator = 148;
const int sym_FunctionName = 149;
const int sym_TypeName = 150;
const int sym_TermName = 151;
const int sym_Keyword = 152;
const int sym_Whitespace = 153;
const int sym_UnknownIdentifier = 154;
const int sym_LookupAny = 155;
const int sym_LookupType = 156;
const int sym_LookupFunction = 157;
const int sym_LookupModule = 158;
const int sym_Untyped = 159;
const int sym_UniformListType = 160;
const int sym_AnonStructType = 161;
const int sym_StructType = 162;
const int sym_NativePatch = 163;
const int sym_PatchBlock = 164;
const int sym_Filesystem = 165;
const int sym_Tarball = 166;
const int sym_Bootstrapping = 167;
const int sym_Done = 168;
const int sym_StorageTypeNull = 169;
const int sym_StorageTypeInt = 170;
const int sym_StorageTypeFloat = 171;
const int sym_StorageTypeBlob = 172;
const int sym_StorageTypeBool = 173;
const int sym_StorageTypeStack = 174;
const int sym_StorageTypeString = 175;
const int sym_StorageTypeList = 176;
const int sym_StorageTypeOpaquePointer = 177;
const int sym_StorageTypeTerm = 178;
const int sym_StorageTypeType = 179;
const int sym_StorageTypeHandle = 180;
const int sym_StorageTypeHashtable = 181;
const int sym_StorageTypeObject = 182;
const int sym_InterfaceType = 183;
const int sym_Delete = 184;
const int sym_Insert = 185;
const int sym_Element = 186;
const int sym_Key = 187;
const int sym_Replace = 188;
const int sym_Append = 189;
const int sym_Truncate = 190;
const int sym_ChangeAppend = 191;
const int sym_ChangeRename = 192;
const int tok_Identifier = 193;
const int tok_ColonString = 194;
const int tok_Integer = 195;
const int tok_HexInteger = 196;
const int tok_Float = 197;
const int tok_String = 198;
const int tok_Color = 199;
const int tok_Bool = 200;
const int tok_LParen = 201;
const int tok_RParen = 202;
const int tok_LBrace = 203;
const int tok_RBrace = 204;
const int tok_LSquare = 205;
const int tok_RSquare = 206;
const int tok_Comma = 207;
const int tok_At = 208;
const int tok_Dot = 209;
const int tok_DotAt = 210;
const int tok_Star = 211;
const int tok_DoubleStar = 212;
const int tok_Question = 213;
const int tok_Slash = 214;
const int tok_DoubleSlash = 215;
const int tok_Plus = 216;
const int tok_Minus = 217;
const int tok_LThan = 218;
const int tok_LThanEq = 219;
const int tok_GThan = 220;
const int tok_GThanEq = 221;
const int tok_Percent = 222;
const int tok_Colon = 223;
const int tok_DoubleColon = 224;
const int tok_DoubleEquals = 225;
const int tok_NotEquals = 226;
const int tok_Equals = 227;
const int tok_PlusEquals = 228;
const int tok_MinusEquals = 229;
const int tok_StarEquals = 230;
const int tok_SlashEquals = 231;
const int tok_ColonEquals = 232;
const int tok_RightArrow = 233;
const int tok_LeftArrow = 234;
const int tok_Ampersand = 235;
const int tok_DoubleAmpersand = 236;
const int tok_VerticalBar = 237;
const int tok_DoubleVerticalBar = 238;
const int tok_Semicolon = 239;
const int tok_TwoDots = 240;
const int tok_Ellipsis = 241;
const int tok_TripleLThan = 242;
const int tok_TripleGThan = 243;
const int tok_Pound = 244;
const int tok_Def = 245;
const int tok_Struct = 246;
const int tok_UnusedName1 = 247;
const int tok_UnusedName2 = 248;
const int tok_UnusedName3 = 249;
const int tok_If = 250;
const int tok_Else = 251;
const int tok_Elif = 252;
const int tok_For = 253;
const int tok_While = 254;
const int tok_State = 255;
const int tok_Return = 256;
const int tok_In = 257;
const int tok_True = 258;
const int tok_False = 259;
const int tok_Namespace = 260;
const int tok_Include = 261;
const int tok_And = 262;
const int tok_Or = 263;
const int tok_Not = 264;
const int tok_Discard = 265;
const int tok_Null = 266;
const int tok_Break = 267;
const int tok_Continue = 268;
const int tok_Switch = 269;
const int tok_Case = 270;
const int tok_Require = 271;
const int tok_Import = 272;
const int tok_Package = 273;
const int tok_Section = 274;
const int tok_Whitespace = 275;
const int tok_Newline = 276;
const int tok_Comment = 277;
const int tok_Eof = 278;
const int tok_Unrecognized = 279;
const int sym_NormalCall = 280;
const int sym_FuncApply = 281;
const int sym_FuncCall = 282;
const int sym_FirstStatIndex = 283;
const int stat_TermCreated = 284;
const int stat_TermPropAdded = 285;
const int stat_TermPropAccess = 286;
const int stat_NameSearch = 287;
const int stat_NameSearchStep = 288;
const int stat_FindModule = 289;
const int stat_Bytecode_WriteTerm = 290;
const int stat_Bytecode_CreateEntry = 291;
const int stat_LoadFrameState = 292;
const int stat_StoreFrameState = 293;
const int stat_AppendMove = 294;
const int stat_GetIndexCopy = 295;
const int stat_GetIndexMove = 296;
const int stat_Interpreter_Step = 297;
const int stat_Interpreter_DynamicMethod_CacheHit = 298;
const int stat_Interpreter_DynamicMethod_SlowLookup = 299;
const int stat_Interpreter_DynamicMethod_SlowLookup_ModuleRef = 300;
const int stat_Interpreter_DynamicMethod_SlowLookup_Hashtable = 301;
const int stat_Interpreter_DynamicMethod_ModuleLookup = 302;
const int stat_Interpreter_DynamicFuncToClosureCall = 303;
const int stat_Interpreter_CopyTermValue = 304;
const int stat_Interpreter_CopyStackValue = 305;
const int stat_Interpreter_MoveStackValue = 306;
const int stat_Interpreter_CopyConst = 307;
const int stat_FindEnvValue = 308;
const int stat_Make = 309;
const int stat_Copy = 310;
const int stat_Cast = 311;
const int stat_ValueCastDispatched = 312;
const int stat_Touch = 313;
const int stat_ListsCreated = 314;
const int stat_ListsGrown = 315;
const int stat_ListSoftCopy = 316;
const int stat_ListDuplicate = 317;
const int stat_ListDuplicate_100Count = 318;
const int stat_ListDuplicate_ElementCopy = 319;
const int stat_ListCast_Touch = 320;
const int stat_ListCast_CastElement = 321;
const int stat_HashtableDuplicate = 322;
const int stat_HashtableDuplicate_Copy = 323;
const int stat_StringCreate = 324;
const int stat_StringDuplicate = 325;
const int stat_StringResizeInPlace = 326;
const int stat_StringResizeCreate = 327;
const int stat_StringSoftCopy = 328;
const int stat_StringToStd = 329;
const int stat_DynamicCall = 330;
const int stat_FinishDynamicCall = 331;
const int stat_DynamicMethodCall = 332;
const int stat_SetIndex = 333;
const int stat_SetField = 334;
const int stat_SetWithSelector_Touch_List = 335;
const int stat_SetWithSelector_Touch_Hashtable = 336;
const int stat_StackPushFrame = 337;
const int sym_LastStatIndex = 338;
const int sym_LastBuiltinName = 339;

const char* builtin_symbol_to_string(int name);
int builtin_symbol_from_string(const char* str);

} // namespace circa

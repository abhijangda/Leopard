using System;
using System.Text.RegularExpressions;
using System.Collections.Generic;

namespace Compiler
{
	public class InterCodeGen
	{
		ASTNode rootNode;
		SymbolTable rootSymTable;

		public InterCodeGen (ASTNode rootNode, SymbolTable symTable)
		{
			this.rootNode = rootNode;
			this.rootSymTable = symTable;
		}

		public string generate ()
		{
			Console.WriteLine ("Generating Intermediate Code\n");
			string s = "";
			s += rootNode.generateCode (rootSymTable, rootSymTable, 0).code;
			return s;
		}
	}

	public class MachineCodeGen
	{
		public static string getIndent (string s)
		{
			for (int i = 0; i < s.Length; i++)
			{
				if (s[i] != ' ')
					return s.Substring (0, i);
			}

			return s;
		}


		public static string getMethodName (string s)
		{
			int j = s.IndexOf (" (", StringComparison.CurrentCultureIgnoreCase);
			string name = "";
			j--;
			while (s[j] != ' ')
			{
				name = s[j] + name;
				j--;
			}

			return name;
		}

		public Type getTypeForString (string s, SymbolTable currSymbolTable, 
			SymbolTable tempSymbolTable, SymbolTable currParamSymTable, out bool isArgument)
		{
			Type t;
			isArgument = false;

			if (Regex.IsMatch (s, @"t\d+"))
			{
				t = tempSymbolTable.getType(s);

				if (t != null)
				{
					return t;
				}
			}

			if (currParamSymTable != null)
			{
				t = currParamSymTable.getType (s);
				if (t != null)
				{
					isArgument = true;
					return t;
				}
			}

			return currSymbolTable.getType (s);
		}
	
		public string getInstructionFromOp (string op)
		{
			if (op == "*")
				return "mul";
			if (op == "+")
				return "add";
			if (op == "/")
				return "div";
			if (op == "-")
				return "sub";
			if (op == "&&")
				return "and";
			if (op == "||")
				return "or";
			if (op == "^")
				return "xor";
			if (op == "==")
				return "eq";
			if (op == "!=")
				return "ne";
			if (op == "<=")
				return "le";
			if (op == ">=")
				return "ge";
			if (op == ">")
				return "gt";
			if (op == ">")
				return "lt";
			return "";
		}

		public string getPushInstructionForConst (ConstantType type, string op)
		{
			if (type.type == Type.Integer)
			{
				return "push.i " + op;
			}
			else if (type.type == Type.Long)
			{
				return "push.l " + op;
			}
			else if (type.type == Type.Double)
			{
				return "push.d " + op;
			}
			else if (type.type == Type.Float)
			{
				return "push.f " + op;
			}
			else if (type.type == Type.Character)
			{
				return "push.c " + op;
			}
			//else if (type.type == Type.String)
			//{
			//}

			return "";
		}

		public string getLocalsStoreInstruction (string op, Dictionary <string, int> dictNumber, 
			Dictionary <string, int> argDictNumber, bool isArgument)
		{
			string instruction = "";

			if (isArgument)
			{
				instruction = "starg " + argDictNumber[op] + "\n";
			}
			else
			{
				instruction = "stloc " + dictNumber[op] + "\n";
			}

			return instruction;
		}

		public string getLocalsLoadInstruction (string op, Dictionary <string, int> dictNumber, 
			Dictionary <string, int> argDictNumber, bool isArgument)
		{
			string instruction = "";

			if (isArgument)
			{
				instruction = "ldarg " + argDictNumber[op] + "\n";
			}
			else
			{
				instruction = "ldloc " + dictNumber[op] + "\n";
			}

			return instruction;
		}
			
		public string getStoreInstructionForMember (string instr, Dictionary <string, int> dictNumber,
			Dictionary <string, int> argDictNumber, bool isArgument, string indent)
		{
			if (instr.Contains ("."))
			{
				/* member of an object */
				string obj = instr.Substring (0, instr.IndexOf ("."));
				string member = instr.Substring (instr.IndexOf (".") + 1);

				if (isArgument)
				{
					/* isArgument is true means object is the argument */
					return indent + "ldarg " + argDictNumber [obj] + "\n" + indent + "stfield " + member + "\n";
				}
				else
				{
					/* object is a local variable */
					return indent + "ldloc " + dictNumber [obj] + "\n" + indent + "stfield " + member + "\n";
				}
			}
			else
			{
				/* member of this */
				return indent + "ldarg 0\n" + indent + "stfield " + instr + "\n";
			}
		}
			
		public string getLoadInstructionForMember (string instr, Dictionary <string, int> dictNumber, 
			Dictionary <string, int> argDictNumber, bool isArgument, string indent)
		{
			if (instr.Contains ("."))
			{
				/* member of an object */
				string obj = instr.Substring (0, instr.IndexOf ("."));
				string member = instr.Substring (instr.IndexOf (".") + 1);

				if (isArgument)
				{
					/* isArgument is true means object is the argument */
					return indent + "ldarg " + argDictNumber [obj] + "\n" + indent + "ldfield " + member + "\n";
				}
				else
				{
					/* object is a local variable */
					return indent + "ldloc " + dictNumber [obj] + "\n" + indent + "ldfield " + member + "\n";
				}
			}
			else
			{
				/* member of this */
				return indent + "ldarg 0\n" + indent + "ldfield " + instr + "\n";
			}
		}

		public string getStoreInstruction (string result, Type typeresult, Dictionary <string, int> dictNumber, 
			Dictionary <string, int> argDictNumber, bool isresultArgument, string indent)
		{
			if (typeresult is MemberType)
			{
				return getStoreInstructionForMember (result, dictNumber, 
					argDictNumber, isresultArgument, indent);
			}
			else if (!(typeresult is TemporaryType))
			{
				return indent + getLocalsStoreInstruction (result, dictNumber, 
					argDictNumber, isresultArgument);
			}
			return "";
		}

		public string getCallInstructionForMethod (string instr, FunctionType type,
			Dictionary <string, int> dictNumber, Dictionary <string, int> argDictNumber, 
			bool isArgument, string indent)
		{
			if (instr.Contains ("."))
			{
				/* member of an object */
				string obj = instr.Substring (0, instr.IndexOf ("."));
				string member = instr.Substring (instr.IndexOf (".") + 1);

				if (isArgument)
				{
					/* isArgument is true means object is the argument */
					return indent + "ldarg " + argDictNumber [obj] + "\n" + indent + "call " + member + "\n";
				}
				else
				{
					/* object is a local variable */
					return indent + "ldloc " + dictNumber [obj] + "\n" + indent + "call " + member + "\n";
				}
			}
			else
			{
				/* member of this */
				return indent + "ldarg 0\n" + indent + "call " + instr + "\n";
			}
		}

		public string getLoadInstruction (Type optype, string op, Dictionary <string, int> dictNumber, 
			Dictionary <string, int> argDictNumber, bool isArgument, string indent)
		{
			if (optype is TemporaryType)
			{
				return "";
			}

			if (optype is ConstantType)
			{
				return indent + getPushInstructionForConst ((ConstantType)optype, op) + "\n";
			}

			if (optype is MemberType)
			{
				return getLoadInstructionForMember (op, dictNumber, argDictNumber,
					isArgument, indent);
			}

			return indent + getLocalsLoadInstruction (op, dictNumber, argDictNumber, isArgument) + "\n";
		}

		public string getConvertInstruction (string type, string indent)
		{
			if (type == "int")
				return indent + "conv.i\n";
			if (type == "short")
				return indent + "conv.s\n";
			if (type == "byte")
				return indent + "conv.b\n";
			if (type == "long")
				return indent + "conv.l\n";
			if (type == "float")
				return indent + "conv.f\n";
			if (type == "double")
				return indent + "conv.d\n";
			if (type == "char")
				return indent + "conv.c\n";
			return indent + "conv " + type + "\n";
		}

		public string genMachineCode (string intercode, SymbolTable rootSymbolTable, SymbolTable tempSymbolTable)
		{
			string machineCode = "";
			int i;
			MatchCollection matches = Regex.Matches (intercode, @".+", RegexOptions.Multiline);
			SymbolTable currSymbolTable = rootSymbolTable;
			i = 0;
			ClassType currClass;
			Dictionary<string, int> dictNumber = null;
			Dictionary<string, int> argDictNumber = null;
			SymbolTable currParamSymTable = null;
			string[] binaryoperators = new string[] {"*", "+", "/", "-", ">>", "<<", "||", "&&", "|", "&"};
			string[] condoperators = new string[] {"==", "<=", ">=", "!="};
			List<string> operators = new List<string> ();
			operators.AddRange (binaryoperators);
			operators.AddRange (condoperators);
		
			bool isArgument;
		
			while (i < matches.Count)
			{
				Match match;

				match = Regex.Match (matches[i].Value, @".class.+");
				string indent = getIndent (matches[i].Value) + "    ";

				if (match.Success) 
				{
					machineCode += matches[i].Value + "\n";
					currClass = (ClassType)rootSymbolTable.getClassType (matches[i].Value.Split (" ".ToCharArray (), 3)[1]);
					currSymbolTable = currClass.symTable;
					while (i < matches.Count && !Regex.IsMatch (matches[i].Value, @"\s*{"))
					{
						i++;
					}

					machineCode += matches [i].Value + "\n";
					if (Regex.IsMatch (matches[i+1].Value, @"\s*}"))
					{
						i++;
						continue;
					}

					machineCode += ".size " + currClass.width + "\n";
				}
				else if (Regex.IsMatch (matches[i].Value, @"\s*{"))
				{
					machineCode += matches[i].Value + "\n";
				}
				else if (Regex.IsMatch (matches[i].Value, @"\s*}"))
				{
					currSymbolTable = currSymbolTable.parent;
					machineCode += matches[i].Value + "\n";
				}
				else if (Regex.IsMatch (matches[i].Value, @"\s*.field.+"))
				{
					machineCode += matches[i].Value + "\n";
				}
				else if (Regex.IsMatch (matches[i].Value, @"\s*.ctor.+"))
				{
					machineCode += matches[i].Value + "\n";
					string constructor = matches[i].Value.Substring (matches[i].Value.IndexOf ("Public") + "Public".Length,
						matches[i].Value.IndexOf ("(") - (matches[i].Value.IndexOf ("Public") + "Public".Length)).Trim ();
					ConstructorType funcType = ((ConstructorType)currSymbolTable.getType (constructor));
					currSymbolTable = funcType.symTable;
					currParamSymTable = funcType.paramSymTable;

					while (i < matches.Count && !Regex.IsMatch (matches[i].Value, @"\s*{"))
					{
						i++;
					}

					machineCode += matches [i].Value + "\n";
					if (Regex.IsMatch (matches[i+1].Value, @"\s*}"))
					{
						i++;
						continue;
					}
					Dictionary<string, Type>.Enumerator e = currSymbolTable.dict.GetEnumerator ();
					dictNumber = new Dictionary<string, int> ();
					machineCode += indent + ".total_locals " + currSymbolTable.dict.Count + "\n";
					int j = 0;
					machineCode += indent + ".locals (\n";
					do
					{
						if (e.Current.Key == null)
						{
							continue;
						}

						if (currParamSymTable.dict.ContainsKey (e.Current.Key))
						{
							continue;
						}

						dictNumber.Add (e.Current.Key, j);
						machineCode += indent + "   " + j + " " + e.Current.Value.ToString () + " " +
							e.Current.Value.width + ",\n";
						j++;
					}
					while (e.MoveNext ());

					argDictNumber = new Dictionary<string, int> ();
					e = currParamSymTable.dict.GetEnumerator ();
					j = 0;

					if (funcType.isStatic)
					{
						j = 1;
					}

					do
					{
						if (e.Current.Key == null)
						{
							continue;
						}

						argDictNumber.Add (e.Current.Key, j);
						j++;
					}
					while (e.MoveNext ());

					machineCode += indent + ")\n";
				}
				else if (Regex.IsMatch (matches[i].Value, @"\s*.method.+"))
				{
					string name = getMethodName (matches[i].Value);
					machineCode += matches[i].Value + "\n";
					FunctionType funcType = (FunctionType)currSymbolTable.getType (name);
					currSymbolTable = funcType.symTable;
					currParamSymTable = funcType.paramSymTable;

					while (i < matches.Count && !Regex.IsMatch (matches[i].Value, @"\s*{"))
					{
						i++;
					}

					machineCode += matches [i].Value + "\n";
					if (Regex.IsMatch (matches[i+1].Value, @"\s*}"))
					{
						i++;
						continue;
					}
					Dictionary<string, Type>.Enumerator e = currSymbolTable.dict.GetEnumerator ();
					dictNumber = new Dictionary<string, int> ();
					machineCode += indent + ".total_locals " + currSymbolTable.dict.Count + "\n";
					int j = 0;
					machineCode += indent + ".locals (\n";
					do
					{
						if (e.Current.Key == null)
						{
							continue;
						}

						if (currParamSymTable.dict.ContainsKey (e.Current.Key))
						{
							continue;
						}

						dictNumber.Add (e.Current.Key, j);
						machineCode += indent + "   " + j + " " + e.Current.Value.ToString () + " " +
							e.Current.Value.width + ",\n";
						j++;
					}
					while (e.MoveNext ());

					argDictNumber = new Dictionary<string, int> ();
					e = currParamSymTable.dict.GetEnumerator ();
					j = 0;

					if (funcType.isStatic)
					{
						j = 1;
					}
				
					do
					{
						if (e.Current.Key == null)
						{
							continue;
						}

						argDictNumber.Add (e.Current.Key, j);
						j++;
					}
					while (e.MoveNext ());

					machineCode += indent + ")\n";
				}
				else if (Regex.IsMatch (matches[i].Value, @"\s*L[\d]+:"))
				{
					machineCode += indent + matches[i].Value + "\n";
				}
				else if (Regex.IsMatch (matches[i].Value, @"[\w\d]+\s*=\s*cast\s*\([\w\d]+\)\s*[\w\d]+"))
				{
					string trimed = matches[i].Value.TrimStart();
					string result = trimed.Substring (0, trimed.IndexOf(" ="));
					string castTo = trimed.Substring (trimed.IndexOf ("(") + 1, trimed.IndexOf (")") - trimed.IndexOf ("(") - 1).Trim ();
					string source = trimed.Substring (trimed.IndexOf ("(")).Trim ();

					Type sourcetype = getTypeForString (source, currSymbolTable, tempSymbolTable,
						currParamSymTable, out isArgument);

					machineCode += getLoadInstruction (sourcetype, source, dictNumber, argDictNumber,
						isArgument, indent);

					machineCode += getConvertInstruction (castTo, indent);

					Type typeresult = getTypeForString (result, currSymbolTable, tempSymbolTable, 
						currParamSymTable, out isArgument);

					//if (typeresult is ConstantType) This can't happen
					machineCode += getStoreInstruction (result, typeresult, dictNumber, argDictNumber, 
						isArgument, indent) + "\n";
				}
				else if (Regex.IsMatch (matches[i].Value, @"\s*iffalse\s*[\w\d]+\s*goto\s*[\w\d]+"))
				{
					//conditional statements
					string trimed = matches[i].Value.TrimStart();
					string source = trimed.Substring ("iffalse".Length, trimed.IndexOf ("goto")).Trim ();
					string label = trimed.Substring (trimed.IndexOf ("goto") + "goto".Length).Trim ();
					Type sourcetype = getTypeForString (source, currSymbolTable, tempSymbolTable,
						currParamSymTable, out isArgument);

					machineCode += getLoadInstruction (sourcetype, source, dictNumber, argDictNumber,
						isArgument, indent);

					machineCode += indent + "brzero " + label + "\n";
				}
				else if (Regex.IsMatch (matches[i].Value, @"\s*goto\s*[\w\d]+"))
				{
					string trimed = matches[i].Value.TrimStart();
					machineCode += indent + trimed.Replace ("goto", "br") + "\n";
				}
				else if (Regex.IsMatch (matches[i].Value, @"\s*[\w\d]+\s*=\s*new\s*[\w\d]+\s*\[\s*\d+\s*\]"))
				{
					string trimed = matches[i].Value.TrimStart();
					string typename = trimed.Substring(trimed.IndexOf("new") + "new".Length, 
						trimed.IndexOf ("[") - trimed.IndexOf("new") - "new".Length).Trim ();
					string result = trimed.Substring(0, trimed.IndexOf(" ="));
					string size = trimed.Substring (trimed.IndexOf ("[") + 1,
						trimed.IndexOf ("]") - trimed.IndexOf ("[") - 1).TrimEnd ();

					Type sizetype = getTypeForString (size, currSymbolTable, tempSymbolTable,
						currParamSymTable, out isArgument);
					machineCode += getLoadInstruction (sizetype, size, dictNumber, argDictNumber,
						isArgument, indent);

					machineCode += indent + "newarr " + typename + "\n";

					Type typeresult = getTypeForString (result, currSymbolTable, tempSymbolTable, 
						currParamSymTable, out isArgument);
					machineCode += getStoreInstruction (result, typeresult, dictNumber, argDictNumber, 
						isArgument, indent) + "\n";
				}
				else if (Regex.IsMatch (matches[i].Value, @"\s*[\w\d]+\s*=\s*new\s*[\w\d]+"))
				{
					//new operator call
					string trimed = matches[i].Value.TrimStart();
					string typename = trimed.Substring(trimed.IndexOf("new") + "new".Length).Trim ();
					string result = trimed.Substring(0, trimed.IndexOf(" ="));
					bool isresultArgument;

					Type functype = currSymbolTable.getConstructorType (typename);
					machineCode += indent + "new " + typename + "\n";

					Type typeresult = getTypeForString (result, currSymbolTable, tempSymbolTable, 
						currParamSymTable, out isresultArgument);
					machineCode += getStoreInstruction (result, typeresult, dictNumber, argDictNumber, 
						isresultArgument, indent) + "\n";
				}
				else if (Regex.IsMatch (matches[i].Value, @"\s*[\w\d]+\s*=\s*call\s*[\w\d]+"))
				{
					//non-void function call
					string trimed = matches[i].Value.TrimStart();
					string func = trimed.Substring(trimed.IndexOf("call") + "call".Length).Trim ();
					string result = trimed.Substring(0, trimed.IndexOf(" ="));
					bool isresultArgument;

					Type functype = getTypeForString (func, currSymbolTable, tempSymbolTable,
						currParamSymTable, out isArgument);
					machineCode += getCallInstructionForMethod (func, (FunctionType)functype, dictNumber, argDictNumber,
						isArgument, indent);
						
					Type typeresult = getTypeForString (result, currSymbolTable, tempSymbolTable, 
						currParamSymTable, out isresultArgument);

					//if (typeresult is ConstantType) This can't happen
					machineCode += getStoreInstruction (result, typeresult, dictNumber, argDictNumber, 
						isresultArgument, indent) + "\n";
				}
				else if (Regex.IsMatch (matches[i].Value, @"\s*param\s*[\w\d]+"))
				{
					//param
					string trimed = matches[i].Value.TrimStart();
					string result_instruction = "";
					string source = trimed.Substring(trimed.IndexOf("param") + "param".Length).Trim ();
					bool isSourceArgument;

					Type sourcetype = getTypeForString (source, currSymbolTable, tempSymbolTable,
						currParamSymTable, out isSourceArgument);
					machineCode += getLoadInstruction (sourcetype, source, dictNumber, argDictNumber,
						isSourceArgument, indent);
				}
				else if (Regex.IsMatch (matches[i].Value, @"\s*call\s*[\w\d]+"))
				{
					//void function call
					string trimed = matches[i].Value.TrimStart();
					string func = trimed.Substring(trimed.IndexOf("call") + "call".Length).Trim ();

					Type functype = getTypeForString (func, currSymbolTable, tempSymbolTable,
						currParamSymTable, out isArgument);
					machineCode += getCallInstructionForMethod (func, (FunctionType)functype, dictNumber, argDictNumber,
						isArgument, indent);
				}
				else if (Regex.IsMatch (matches[i].Value, @"\s*[\w\d]+\s*=\s*[\w\d]+\s*\[[\w\d]+\]"))
				{
					string trimed = matches[i].Value.TrimStart();
					string result_instruction = "";
					string result = trimed.Substring(0, trimed.IndexOf(" ="));
					string source_array = trimed.Substring(trimed.IndexOf ("=") + 1, trimed.IndexOf("[") - trimed.IndexOf ("=") - 1).Trim ();
					string source_index = trimed.Substring(trimed.IndexOf("[") + 1, 
						trimed.IndexOf("]") - trimed.IndexOf("[") - 1);
					bool isresultArgument, isSourceArgument;

					Type typesource = getTypeForString (source_array, currSymbolTable, tempSymbolTable, 
						currParamSymTable, out isresultArgument);
					machineCode += getLoadInstruction (typesource, source_array, dictNumber, argDictNumber, 
						isresultArgument, indent) + "\n";

					Type typesource_index = getTypeForString (source_index, currSymbolTable, tempSymbolTable, 
						currParamSymTable, out isresultArgument);
					machineCode += getLoadInstruction (typesource_index, source_index, dictNumber, argDictNumber,
						isresultArgument, indent);

					machineCode += indent + "ldelem\n";
	
					Type resulttype = getTypeForString (result, currSymbolTable, tempSymbolTable,
						currParamSymTable, out isSourceArgument);
					machineCode += getStoreInstruction (result, resulttype, dictNumber, argDictNumber,
						isSourceArgument, indent);
				}
				else if (Regex.IsMatch (matches[i].Value, @"\s*[\w\d]+\s*\[[\w\d]+\]\s*=\s*[\w\d]+"))
				{
					string trimed = matches[i].Value.TrimStart();
					string result_instruction = "";
					string result = trimed.Substring(0, trimed.IndexOf(" ="));
					string result_array = result.Substring(0, result.IndexOf("["));
					string result_index = result.Substring(result.IndexOf("[") + 1, 
						result.IndexOf("]") - result.IndexOf("[") - 1);
					string source = trimed.Substring(trimed.IndexOf("=") + 1).Trim ();
					bool isresultArgument, isSourceArgument;

					Type typeresult = getTypeForString (result_array, currSymbolTable, tempSymbolTable, 
						currParamSymTable, out isresultArgument);
					machineCode += getLoadInstruction (typeresult, result_array, dictNumber, argDictNumber, 
						isresultArgument, indent) + "\n";

					Type typeresult_index = getTypeForString (result_index, currSymbolTable, tempSymbolTable, 
						currParamSymTable, out isresultArgument);
					machineCode += getLoadInstruction (typeresult_index, result_index, dictNumber, argDictNumber,
						isresultArgument, indent);

					Type sourcetype = getTypeForString (source, currSymbolTable, tempSymbolTable,
						currParamSymTable, out isSourceArgument);
					machineCode += getLoadInstruction (sourcetype, source, dictNumber, argDictNumber,
						isSourceArgument, indent);

					machineCode += indent + "stelem\n";
				}
				else if (Regex.IsMatch (matches[i].Value, @"\s*[\w\d\.]+\s*=\s*[\w\d\.]+$"))
				{
					string trimed = matches[i].Value.TrimStart();
					string result_instruction = "";
					string result = trimed.Substring(0, trimed.IndexOf(" ="));
					string source = trimed.Substring(trimed.IndexOf("=") + 1).Trim ();
					bool isresultArgument, isSourceArgument;

					Type sourcetype = getTypeForString (source, currSymbolTable, tempSymbolTable,
						currParamSymTable, out isSourceArgument);
					machineCode += getLoadInstruction (sourcetype, source, dictNumber, argDictNumber,
						isSourceArgument, indent);
				
					Type typeresult = getTypeForString (result, currSymbolTable, tempSymbolTable, 
						currParamSymTable, out isresultArgument);

					//if (typeresult is ConstantType) This can't happen
					result_instruction = getStoreInstruction (result, typeresult, dictNumber, argDictNumber, 
						isresultArgument, indent);

					machineCode += result_instruction + "\n";
				}
				else
				{
					foreach (string op in operators)
					{
						//Include case when an array element is set
						if (Regex.IsMatch (matches[i].Value, @"\s*[\w\d]+\s*=\s*[\w\d]+\s*\" + op + @"\s*[\w\d]+$"))
						{
							string instruction = getInstructionFromOp (op);
							string trimed = matches[i].Value.TrimStart();
							string result = trimed.Substring(0, trimed.IndexOf(" ="));
							string op1 = trimed.Substring(trimed.IndexOf("=") + 1,
								trimed.IndexOf(op) - (trimed.IndexOf("=") + 1)).Trim ();
							string op2 = trimed.Substring(trimed.IndexOf(op) + 1).Trim ();
							Type type1 = getTypeForString (op1, currSymbolTable, tempSymbolTable,
															currParamSymTable, out isArgument);
							string op1instruction = "";
							string op2instruction = "";

							op1instruction = getLoadInstruction (type1, op1, dictNumber, argDictNumber,
								isArgument, indent);

							Type type2 = getTypeForString (op2, currSymbolTable, tempSymbolTable, 
								currParamSymTable, out isArgument);

							op2instruction = getLoadInstruction (type2, op2, dictNumber, argDictNumber,
								isArgument, indent);
								
							Type typeresult = getTypeForString (result, currSymbolTable, tempSymbolTable,
								currParamSymTable, out isArgument);

							string result_instruction = "";

							result_instruction = getStoreInstruction (result, typeresult, dictNumber, 
								argDictNumber, isArgument, indent);

							machineCode += op1instruction + "\n" + op2instruction + "\n" + indent + instruction + "\n" +
								result_instruction;

							break;
						}
					}
				}

				i++;
			}
		
			return machineCode;
		}
	}
}
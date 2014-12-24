using System;
using System.Collections.Generic;
using System.Text.RegularExpressions;
using System.IO;
using System.Reflection;

namespace VMAssembler
{
	public class VMAssembler
	{
		Dictionary<string, byte> dictByteCodes;

		public static byte[] ConvertStringToBytes (string s)
		{
			List<byte> listBytes = new List<byte> ();

			foreach (char c in s)
			{
				listBytes.Add (BitConverter.GetBytes (c) [0]);
			}

			return listBytes.ToArray ();
		}

		public static byte[] ConvertToByteList (long value)
		{
			return BitConverter.GetBytes (value);
		}

		public static byte[] ConvertToByteList (int value)
		{
			return BitConverter.GetBytes (value);
		}

		public static byte[] ConvertToByteList (short value)
		{
			return ConvertStringToBytes(value.ToString ());
		}

		public static byte[] ConvertToByteList (float value)
		{
			return BitConverter.GetBytes (value);
		}

		public static byte[] ConvertToByteList (double value)
		{
			return BitConverter.GetBytes (value);
		}

		public static byte[] ConvertToByteList (char value)
		{
			return BitConverter.GetBytes (value);
		}

		public static string getType (string value)
		{
			byte bvalue;
			short svalue;
			int ivalue;
			long lvalue;
			double dvalue;
			float fvalue;
			char cvalue;

			if (char.TryParse (value, out cvalue))
				return "char";

			if (short.TryParse (value, out svalue))
				return "short";
				
			if (int.TryParse (value, out ivalue))
				return "int";

			if (long.TryParse (value, out lvalue))
				return "long";

			if (float.TryParse (value, out fvalue))
				return "float";

			if (double.TryParse (value, out dvalue))
				return "double";

			return "string";
		}

		public List<byte> getCode (string assembly)
		{
			int i = 0;
			MatchCollection matches = Regex.Matches (assembly, @".+", RegexOptions.Multiline);
			MethodInfo entryPoint = null;
			ClassInfo mainClass = null;
			List<ClassInfo> classInfo = new List<ClassInfo> ();

			/* Read the code and arrange data */
			while (i < matches.Count)
			{
				string line = matches [i].Value;

				if (Regex.IsMatch (line, @"\s*.class.+"))
				{
					string name, parent;
					int size = 0;
					List<MemberInfo> listMemberInfo;
					List<MethodInfo> listMethodInfo;
					int l = line.IndexOf (".class") + ".class".Length;
					int countbrace = 0;
					bool isClassEntry = false;

					listMemberInfo = new List<MemberInfo> ();
					listMethodInfo = new List<MethodInfo> ();
					name = line.Substring (l, line.IndexOf ("extends") - l).Trim ();
					l = line.IndexOf ("extends") + "extends".Length;
					parent = line.Substring (l).Trim ();

					while (i < matches.Count && matches[i].Value.Trim () != "{")
					{
						i++;
					}

					countbrace += 1;

					while  (i < matches.Count && countbrace != 0)
					{
						line = matches [i].Value.Trim ();

						if (line == "{")
							countbrace += 1;
						else if (line == "}")
							countbrace -= 1;

						if (Regex.IsMatch (line, @"\s*.field.+"))
						{
							string fieldname, fieldtype, accessSpec;
							bool isStatic;
							MethodInfo.AccessSpecifer accessSpecInt = MemberInfo.AccessSpecifer.Public;

							string[] split = line.Split (" ".ToCharArray (), 5);
							accessSpec = split[0];

							if (Regex.IsMatch (line, @"\bstatic\b"))
							{
								isStatic = true;
								fieldtype = split [2];
								fieldname = split [3];
							}
							else
							{
								isStatic = false;
								fieldtype = split [1];
								fieldname = split [2];
							}

							foreach (string s in Enum.GetNames (typeof (MethodInfo.AccessSpecifer)))
							{
								if (s.ToLower () == accessSpec.ToLower ())
								{
									accessSpecInt = (MethodInfo.AccessSpecifer)Enum.Parse (typeof (MethodInfo.AccessSpecifer), accessSpec);
									break;
								}
							}

							listMemberInfo.Add (new MemberInfo (fieldname, fieldtype, isStatic, accessSpecInt));
						}
						else if (Regex.IsMatch (line, @"\s*.method.+"))
						{
							string fieldname, fieldtype, accessSpec;
							bool isStatic;
							MethodInfo.AccessSpecifer accessSpecInt;
							MethodCode code;
							int totalLocals = 0;
							List<MethodCode.Local> listLocals;
							List<MethodCode.Instruction> listInstructions;
							List<string> listArgsType;

							listInstructions = new List<MethodCode.Instruction> ();
							listLocals = new List<MethodCode.Local> ();
							listArgsType = new List<string> ();
							string args = line.Substring (line.IndexOf ("("));
							string sig = line.Substring (0, line.IndexOf ("(")).Trim ();

							if (line.Contains (".entrypoint"))
							{
								string[] split;

								split = sig.Split (" ".ToCharArray (), 7);
								isStatic = true;
								accessSpec = split [2];
								fieldtype = split [4];
								fieldname = split [5];
							}
							else if (Regex.IsMatch (line, @"\bstatic\b"))
							{
								string[] split;

								split = sig.Split (" ".ToCharArray (), 6);
								isStatic = true;
								accessSpec = split [1];
								fieldtype = split [3];
								fieldname = split [4];
							}
							else 
							{
								string[] split;

								split = sig.Split (" ".ToCharArray (), 5);
								isStatic = false;
								accessSpec = split [1];
								fieldtype = split [2];
								fieldname = split [3];
							}

							args = args.Replace ('(', ' ').Replace (')', ' ').Trim ();

							foreach (string arg in args.Split (",".ToCharArray (), StringSplitOptions.RemoveEmptyEntries))
							{
								listArgsType.Add (arg.Trim ());
							}
								
							if (line.Contains (".entrypoint"))
							{
								isClassEntry = true;
							}

							i += 1;

							while (i < matches.Count && matches[i].Value.Trim () != "}")
							{
								line = matches [i].Value.Trim ();

								if (line == "{")
								{
									countbrace += 1;
									i++;
									continue;
								}
								else if (line == "}")
								{
									countbrace -= 1;
									i++;
									continue;
								}

								if (Regex.IsMatch (line, @"\s*.total_locals"))
								{
									totalLocals = int.Parse (line.Split (" ".ToCharArray (), 3)[1]);
								}
								else if (Regex.IsMatch (line, @"\s*.locals\s*\("))
								{
									i++;

									while  (i < matches.Count && matches[i].Value.Trim () != ")")
									{
										int n, s;
										string type;

										line = matches [i].Value.Trim ();
										line = line.Replace (",", "");
										string[] split = line.Split (" ".ToCharArray (), 3);

										n = int.Parse (split[0]);
										type = split[1];
										s = int.Parse (split [2]);

										listLocals.Add (new MethodCode.Local (n, s, type));
										i += 1;
									}
								}
								else if (Regex.IsMatch (line, @"\s*\w[\w\d]*\:"))
								{
									MethodCode.Instruction instruction = new MethodCode.Instruction ();

									instruction.Add (100);
									byte[] listopbytes;
									line = line.Trim ();
									line = line.Substring (0, line.Length - 1);
									listopbytes = ConvertStringToBytes (line.Trim ());
									instruction.Add ((byte)listopbytes.Length);
									instruction.AddRange (listopbytes);
									listInstructions.Add (instruction);
								}
								else
								{
									string instr;
									string op;
									byte byteCode;
									string[] split;
									byte[] listopbytes = null;

									split = line.Split (" ".ToCharArray (), 3);
									instr = split[0];
									byteCode = dictByteCodes [instr];


									MethodCode.Instruction instruction = new MethodCode.Instruction ();
									instruction.Add (byteCode);
									if (split.Length == 2 && split[1] != "")
									{
										op = split [1];
										listopbytes = ConvertStringToBytes (op);
										instruction.Add ((byte)listopbytes.Length);
										instruction.AddRange (listopbytes);
									}
									else
									{
										instruction.Add ((byte)0);
									}

									listInstructions.Add (instruction);
								}

								i++;

								countbrace -= 1;
							}

							code = new MethodCode (totalLocals, listLocals, listInstructions);
							listMethodInfo.Add (new MethodInfo (fieldname, fieldtype, isStatic, 
								MethodInfo.StringToAccessSpecifier (accessSpec), listArgsType, code));

							if (isClassEntry)
								entryPoint = listMethodInfo[listMethodInfo.Count - 1];
						}
						else if (Regex.IsMatch (line, @"\s*.ctor.+"))
						{

						}

						i++;
					}

					classInfo.Add (new ClassInfo (name, size, parent, (byte)listMemberInfo.Count, 
						(byte)listMethodInfo.Count, listMethodInfo, listMemberInfo));

					if (isClassEntry)
						mainClass = classInfo[classInfo.Count - 1];
				}

				i++;
			}

			/* Write Code */
			List<byte> finalCode = new List<byte> ();
			if (BitConverter.IsLittleEndian)
			{
				finalCode.Add (1);
			}
			else
			{
				finalCode.Add (0);
			}

			finalCode.Add ((byte)mainClass.name.Length);
			finalCode.AddRange (ConvertStringToBytes (mainClass.name));
			finalCode.Add ((byte)entryPoint.name.Length);
			finalCode.AddRange (ConvertStringToBytes (entryPoint.name));
			finalCode.Add ((byte)classInfo.Count);

			foreach (ClassInfo cinfo in classInfo)
			{
				List<byte> cinfoBinary = cinfo.toBinary ();
				finalCode.AddRange (cinfoBinary);
			}

			return finalCode ;
		}

		public void getByteCodeandInstr (string s, out byte code, out string instr)
		{
			int i;
			for (i = 0; i < s.Length; i++)
			{
				if (!Char.IsDigit (s[i]))
					break;
			}

			code = byte.Parse (s.Substring (0, i + 1));

			int j;
			for (j = i + 1; j < s.Length; j++)
			{
				if (s[j] == ' ')
					break;
			}

			instr = s.Substring (i, j-i).Trim ();
		}

		public VMAssembler (string byteCodeFile)
		{
			string byteCodeFormat = File.ReadAllText (byteCodeFile);
			MatchCollection matches = Regex.Matches (byteCodeFormat, @".+", RegexOptions.Multiline);

			//Add to code the endianness also BitConverter.IsLittleEndian;
			dictByteCodes = new Dictionary<string, byte> ();

			foreach (Match match in matches)
			{
				byte code;
				string instr;

				getByteCodeandInstr (match.Value, out code, out instr);
				if (instr != "")
					dictByteCodes.Add (instr, code);
			}
		}
	}
}
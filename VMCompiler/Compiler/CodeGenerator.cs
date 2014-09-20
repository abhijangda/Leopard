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

	public class VariableDescriptor
	{
		public string name {get; set;}
		public int size {get; set;}
		public string value;
		public Type type;

		public VariableDescriptor (string name, int size, Type type,
		                           string value = "")
		{
			this.name = name;
			this.size = size;
			this.value = value;
			this.type = type;
		}

		public string get_loc ()
		{
			return "";
		}
	}

	public class TemporaryDescriptor : VariableDescriptor
	{
		public TemporaryDescriptor (string name, int size, Type type,
		                            string value = "") : base (name, size, type, value)
		{
		}
	}

	public class LocalDescriptor : VariableDescriptor
	{
		public int number {get; private set;}
		public LocalDescriptor (int number, string name, int size, Type type,
		                        string value = "") : base (name, size, type, value)
		{
			this.number = number;
		}
	}

	public class MachineCodeGen
	{
		string intercode;
		Stack<Dictionary<string, VariableDescriptor>> vd_stack;
		Dictionary <string, VariableDescriptor> vd_dict;
	
		public MachineCodeGen (string intercode)
		{
			this.intercode = intercode;
			vd_stack = new Stack<Dictionary <string, VariableDescriptor>> ();
		}

		public void push_vd ()
		{
			vd_dict = new Dictionary<string, VariableDescriptor> ();
			vd_stack.Push (vd_dict);
		}

		public void pop_vd ()
		{
			vd_stack.Pop ();
			vd_dict = vd_stack.Peek ();
		}

		public string genMachineCode (SymbolTable symTable)
		{
			string machineCode = "";
			MatchCollection mc = Regex.Matches (intercode, @".struct.+\}", 
			                                    RegexOptions.Singleline);
			foreach (Match m in mc)
			{
				machineCode += m.Value + "\n";
				intercode = intercode.Replace (m.Value, "");
			}

			MatchCollection linesmc = Regex.Matches (intercode, @".+", 
			                                         RegexOptions.Multiline);
			int iter = -1;
			//while (++iter < linesmc.Count)
			//	Console.WriteLine (linesmc [iter].Value);
			iter = 0;
			do
			{
				Match m = Regex.Match (linesmc[iter].Value, @"^extern\s[\w_][\w_\d]+");
				if (m.Success)
					machineCode += "\t" + m.Value + "\n";
				else
				{
					iter--;
					if (iter == -1)
						iter = 0;
					break;
				}
				iter ++;
				if (iter >= linesmc.Count)
				{
					iter = 0;
					break;
				}
			}
			while (true);

			SymbolTable curSymTable = null;
			do
			{
				Match m = null;
				string[] operators = new string [] {"+", "*", "/", "-"};
				int size = 4;

				/* Function definitions */
				m = Regex.Match (linesmc [iter].Value, @"func\s+\w[\w\d]*:");
				if (m.Success)
				{
					string func = m.Value.Substring ("func".Length, 
					                                 m.Value.Length - "func".Length - 1).Trim ();
					//curSymTable = symTable.getFuncTable (func);
					machineCode += "\t.locals\n\t{";
					iter++;
					continue;
				}

				/*Return statement*/
				m = Regex.Match (linesmc [iter].Value, @"return\s*[\w\d]*");
				if (m.Success)
				{
				}

				foreach (string op in operators)
				{
					m = Regex.Match (linesmc[iter].Value, @"[\w\d]+\s*=\s*[\w\d]+\s*\" + op + @"\s*[\w\d]+");

					if (m.Success)
					{

						break;
					}
				}

				if (m != null && m.Success)
				{
					iter ++;
					continue;
				}

				m = Regex.Match (linesmc[iter].Value, @"[\w\d]+\s*=\s*new\s*[\w\d]+");
				if (m.Success)
				{

					iter++;
					continue;
				}

				m = Regex.Match (linesmc[iter].Value, @"[\w\d]+\s*=\s*[\w\d]+\.[\w\d]");
				if (m.Success)
				{

					iter++;
					continue;
				}

				m = Regex.Match (linesmc[iter].Value, @"[\w\d]+\.[\w\d]\s*=\s*[\w\d]+");
				if (m.Success)
				{
					iter++;
					continue;
				}
		
				m = Regex.Match (linesmc[iter].Value, @"[\w\d]+\s*=\s*[\w\d]+");
				if (m.Success)
				{

					iter++;
					continue;
				}

				/*For Strings*/
				m = Regex.Match (linesmc[iter].Value, "[\\w\\d]+\\s*=\\s*\".+\"");
				if (m.Success)
				{

				}

				m = Regex.Match (linesmc[iter].Value, @"IfFalse\s+[\w\d]+\s+goto\s+[\w\d]+");
				if (m.Success)
				{
					string op = m.Value.Substring ("ifFalse".Length, m.Value.IndexOf ("goto") - "ifFalse".Length).Trim ();
					VariableDescriptor vd = vd_dict [op];
					machineCode += "\tsub " + vd.get_loc () + ", 0\n";
					string label = m.Value.Substring (m.Value.IndexOf ("goto") + "goto".Length).Trim ();
					machineCode += "\tjne " + label + "\n";
					iter++;
					continue;
				}

				m = Regex.Match (linesmc[iter].Value, @"goto\s+[\w\d]+");
				if (m.Success)
				{
					string label = m.Value.Substring (m.Value.IndexOf ("goto") + "goto".Length).Trim ();
					machineCode += "\tjmp " + label + "\n";
					iter++;
					continue;
				}

				m = Regex.Match (linesmc [iter].Value, @"call\s+\w[\w\d]+");
				if (m.Success)
				{
				}

				iter ++;
			}
			while (iter < linesmc.Count);

			return machineCode;
		}


	}
}
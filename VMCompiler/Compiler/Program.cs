using System;
using System.IO;
using LanguageGrammar;
using System.Collections.Generic;
using VMAssembler;

/*
TODO: May have to include types in instruction also like stdarg.i4 or call p (int32, int64)
*/

namespace Compiler
{
	class MainClass
	{
		public static short countInString (string str1, string str2)
		{
			short count = 0;
			int index = -1;
			index = str1.IndexOf (str2, index + 1);
			if (index == -1)
				return 0;
			while (index != -1)
			{
				count += 1;
				index = str1.IndexOf (str2, index + 1);
			}

			return count;
		}

		public static void Main (string[] args)
		{
			string file = "./../../test";

			if (args.Length >= 1)
				file = args[0];
	
			/*Parser p = new Parser (file);
			InterCodeGen intercodegen = new InterCodeGen (p.startParsing (), p.symTableTree);
			string intercode = intercodegen.generate ();
			//Console.WriteLine ("\n" + intercode + "\n");
			string machineCode = new MachineCodeGen ().genMachineCode (intercode, p.symTableTree, ASTNode.tempsSymTable);
			Console.WriteLine ("LeapordASM Code Generated \n" +  machineCode);
			*/
			string machineCode = ".class A extends None \n{\n.size 8\n .method .entrypoint Public static void main ()\n {\n }\n}\n";
			VMAssembler.VMAssembler assembler = new VMAssembler.VMAssembler ("./../../../byte_code_specification_stack");
			List<byte> code = assembler.getCode (machineCode);
			string output = "./../../output";
			FileStream stream = File.Open (output, FileMode.OpenOrCreate);
			stream.Write (code.ToArray (), 0, code.Count);
		}
	}
}
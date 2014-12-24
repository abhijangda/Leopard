using System;
using System.IO;
using LanguageGrammar;
using System.Collections.Generic;
using VMAssembler;

/*
TODO: May have to include types in instruction also like stdarg.i4 or call p (int32, int64)
TODO: Correct if (j == 0) like instructions.
TODO: Improve semantic analyzer.
TODO: Add ability to convert from byte to short to int to long to float to double in semantic analyzer
TODO: Add Constructor in assembly
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
			string output = "./../../output";

			if (args.Length >= 1)
			{
				file = args[1];
				output = args[2];
			}
	
			Parser p = new Parser (file);
			InterCodeGen intercodegen = new InterCodeGen (p.startParsing (), p.symTableTree);
			string intercode = intercodegen.generate ();
			//Console.WriteLine ("\n" + intercode + "\n");
			string machineCode = new MachineCodeGen ().genMachineCode (intercode, p.symTableTree, ASTNode.tempsSymTable);
			Console.WriteLine ("LeapordASM Code Generated \n" +  machineCode);

			VMAssembler.VMAssembler assembler = new VMAssembler.VMAssembler ("./../../../byte_code_specification_stack");
			List<byte> code = assembler.getCode (machineCode);
			File.Delete (output);
			FileStream stream = File.Open (output, FileMode.OpenOrCreate);
			byte[] arr = code.ToArray ();
			stream.Write (arr, 0, code.Count);
			stream.Close ();
		}
	}
}
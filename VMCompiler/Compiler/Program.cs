using System;
using System.IO;
using LanguageGrammar;
using System.Collections.Generic;

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
	
			Parser p = new Parser (file);
			InterCodeGen intercodegen = new InterCodeGen (p.startParsing (), p.symTableTree);
			string intercode = intercodegen.generate ();
			Console.WriteLine ("\n" + intercode + "\n");
			Console.WriteLine ("LeapordASM Code Generated \n" +  new MachineCodeGen ().genMachineCode (intercode, p.symTableTree, ASTNode.tempsSymTable));
		}
	}
}
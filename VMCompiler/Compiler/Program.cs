using System;
using System.IO;
using LanguageGrammar;
using System.Collections.Generic;
using VMAssembler;
using System.Runtime.Serialization.Formatters.Binary;

/*
TODO: May have to include types in instruction also like stdarg.i4 or call p (int32, int64)
TODO: Correct if (j == 0) like instructions.
TODO: Improve semantic analyzer.
TODO: Add ability to convert from byte to short to int to long to float to double in semantic analyzer
TODO: Add Constructor in assembly
TODO: Test if correct bytecode is produced for following code
class P { public int k;}
class Q { public P p;}
class M { public main () { M m = new M (); m.p.k = 10;}}
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
			string parser = "./../../parser";

			if (args.Length >= 1)
			{
				file = args[1];
				output = args[2];
			}
	
			Parser p;

			if (File.Exists (parser))
			{
				Stream TestFileStream = File.OpenRead(parser);
				BinaryFormatter deserializer = new BinaryFormatter();
				p = (Parser)deserializer.Deserialize(TestFileStream);
				TestFileStream.Close();
			}
			else
			{
				p = new Parser ();
				Stream TestFileStream = File.Create(parser);
				BinaryFormatter serializer = new BinaryFormatter();
				serializer.Serialize(TestFileStream, p);
				TestFileStream.Close();
			}


			InterCodeGen intercodegen = new InterCodeGen (p.startParsing (file), p.symTableTree);
			string intercode = intercodegen.generate ();
			//Console.WriteLine ("\n" + intercode + "\n");
			string machineCode = new MachineCodeGen ().genMachineCode (intercode, p.symTableTree, ASTNode.tempsSymTable);
			Console.WriteLine ("LeapordASM Code Generated \n" +  machineCode);

			//string machineCode = ".class B extends None\n{\n    .size 8\n    .field Public int i\n    .field Public int g\n}\n.class A extends None\n{\n.size 16\n.field Public B p\n.field Public int b\n .field Public int c\n.method .entrypoint Public static void main ()\n{\n     .total_locals 1\n     .locals (\n       0 A 8,\n       )\n      new A\n stloc 0\n push.i 90\nldloc 0\n ldfield A.p\n stfield B.i\n ldloc 0\n ldfield A.p\n ldfield B.i\n}\n}";
			//string machineCode = ".class A extends None\n{\n.size 8\n.method .entrypoint Public static void main ()\n{\n     .total_locals 1\n     .locals (\n       0 A 8,\n       )\n     new A\n     }\n}";
			//string machineCode = ".class A extends None \n{\n.size 8\n .method .entrypoint Public static void main ()\n {\n     .total_locals 1\n     .locals (\n        0 int[10] 8,\n     )\n       push.i 10\n       newarr int\n\n       stloc 0\n\n       ldloc 0\n\n\n       push.i 0\n  push.i 98\n     stelem\n ldloc 0\n push.i 1\n push.i 34\n stelem\n ldloc 0\n push.i 2\n push.i 34\n stelem\n}\n}\n";
			//string machineCode = ".class A extends None \n{\n.size 8\n .method .entrypoint Public static void main ()\n {\n     .total_locals 1\n     .locals (\n        0 int[10] 8,\n     )\n       push.i 10\n       newarr int\n\n       stloc 0\n\n       ldloc 0\n\n\n       push.i 0\n  push.i 98\n     stelem\n ldloc 0\n push.i 1\n push.i 34\n stelem\n ldloc 0\n push.i 0\n ldelem\n}\n}\n";
			//string machineCode = ".class A extends None \n{\n.size 8\n .method .entrypoint Public static void main ()\n {\n     .total_locals 1\n     .locals (\n        0 int 4,\n    )\n  push.l 10\n newarr int\n }\n}\n";
			//string machineCode = ".class A extends None \n{\n.size 8\n .method .entrypoint Public static void main ()\n {\n     .total_locals 1\n     .locals (\n        0 int 4,\n     )\n  push.i 10\n}\n}\n";
			//string machineCode = ".class A extends None \n{\n.size 8\n .method .entrypoint Public static void main ()\n {\n     .total_locals 1\n     .locals (\n        0 int 4,\n     )\n      push.i 8\n  push.i 9\n br L\n sub\n push.i 10\n push.i 11\n L:\n add\n}\n}\n";
			//string machineCode = ".class A extends None \n{\n.size 8\n .method .entrypoint Public static void main ()\n {\n     .total_locals 1\n     .locals (\n        0 int 4,\n     )\n      push.i 8\n  push.i 9\n br L\n sub\n push.i 13\n br L1\n push.i 10\n push.i 11\n L1:\n push.i 12\n L:\n push.i 15\n}\n}\n";
			//string machineCode = ".class A extends None \n{\n.size 8\n .method .entrypoint Public static void main ()\n {\n     .total_locals 1\n     .locals (\n        0 int 4,\n     )\n      push.i 8\n  push.i 9\n push.i 10\n push.i 11\n push.i 12\n push.i 13\n push.i 14\n push.i 15\n push.i 16\n mul\n}\n}\n";
			//string machineCode = ".class A extends None \n{\n.size 8\n .method .entrypoint Public static void main ()\n {\n     .total_locals 1\n     .locals (\n        0 int 4,\n     )\n      push.i 1\n\n      push.i 2\n\n      mul\n      push.i 0\n\n\n      add\n      push.i 3\n\n      push.i 4\n\n      mul\n\n\n      sub\n\n      push.i 5\n\n      add\n      stloc 0\n\n }\n}\n";
			//string machineCode = ".class A extends None \n{\n.size 8\n .method .entrypoint Public static void main ()\n {\n     .total_locals 1\n     .locals (\n        0 int 4,\n     )\n      push.i 1\n\n      push.i 2\n\n      mul\n\n      push.i 5\n\n      add\n      stloc 0\n\n }\n}\n";
			//string machineCode = ".class A extends None \n{\n.size 8\n .method .entrypoint Public static void main ()\n {\n     .total_locals 1\n     .locals (\n        0 int 4,\n     )\n      push.i 5\n      push.i 1\n      sub\n      stloc 0\n\n }\n}\n";
			//string machineCode = ".class A extends None \n{\n.size 8\n .method .entrypoint Public static void main ()\n {\n     .total_locals 1\n     .locals (\n        0 int 4,\n     )\n      push.i 1\n\n      push.i 2\n\n      mul\n      push.i 0\n\n\n      add\n      push.i 3\n\n      push.i 4\n\n      mul\n\n\n      sub\n      push.i 5\n\n      push.i 8\n\n      mul\n\n\n      add\n\n      push.i 9\n\n      add\n\n      push.i 10\n\n      add\n\n      push.i 11\n\n      add\n\n      push.i 12\n\n      add\n      stloc 0\n\n }\n}\n";
			//string machineCode = ".class A extends None \n{\n.size 8\n .method .entrypoint Public static void main ()\n {\n     .total_locals 1\n     .locals (\n        0 int 4,\n     )\n      push.i 1\n\n      push.i 2\n\n      add\n\n      push.i 3\n\n      add\n\n      push.i 4\n\n      add\n\n      push.i 5\n\n      add\n\n      push.i 6\n\n      add\n\n      push.i 7\n\n      add\n\n      push.i 8\n\n      add\n\n      push.i 9\n\n      add\n\n      push.i 10\n\n      add\n\n      push.i 11\n\n      add\n      stloc 0\n\n }\n}\n\n";
			//string machineCode = ".class A extends None \n{\n.size 8\n .method .entrypoint Public static void main ()\n {\n     .total_locals 1\n     .locals (\n        0 int 4,\n     )\n      push.d 10.11\n\n      push.d 11.123\n\n      add\n      stloc 0\n\n }\n}\n\n";
			//string machineCode = ".class A extends None \n{\n.size 8\n .method .entrypoint Public static void main ()\n {\n     .total_locals 1\n     .locals (\n        0 float 4,\n     )\n      push.d 8.3\n\n      push.d 9.1\n\n      mul\n      push.d 9.8\n\n\n      add\n\n      push.d 2.3\n\n      add\n\n      push.d 3.2\n\n      sub\n      stloc 0\n\n }\n}\n";
			//string machineCode = ".class A extends None \n{\n.size 8\n .method .entrypoint Public static void main ()\n {\n     .total_locals 1\n     .locals (\n        0 float 4,\n     )\n       push.i 0\n       stloc 0\n\n          ldloc 0\n\n\n          push.i 0\n\n          eq\n        brzero L1\n            L1:\n }\n}\n";
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
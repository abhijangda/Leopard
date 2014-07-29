using System;
using System.Collections.Generic;

namespace Compiler
{
	public class FunctionType : Type
	{
		public List<Type> listParamTypes;
		public Type returnType;
		public SymbolTable symTable {get; private set;}
		public FunctionType (Type return_type, SymbolTable symTable,
		                     ParameterListNode paramList) : base (return_type.width)
		{
			returnType = return_type;
			this.symTable = symTable;
			if (paramList != null)
			{
				foreach (ASTNode node in paramList.listNodes)
				{
					symTable.dict.Add (((ParameterNode)node).id, 
					                   ((ParameterNode)node).type);
				}
			}
		}
	}

	public class ArrayType : Type
	{
		public Type type;

		public ArrayType (Type type, int width) : base (type.width * width)
		{
			this.type = type;
			this.width = width;
		}
	}

	public class ClassType : Type
	{
		public SymbolTable symTable {get; private set;}
		public string name {get; private set;}
		public override int width 
		{
			get 
			{
				return symTable.getTotalSize ();
			}
			set 
			{
				base.width = value;
			}
		}
		public ClassType (string name, SymbolTable classSymTable) : base (name, Tag.ClassType, 0)
		{
			symTable = classSymTable;
			this.name = name;
		}
	}

	public class SymbolTable
	{
		public SymbolTable parent{get; private set;}
		public List<SymbolTable> children{get; private set;}
		public Dictionary <string, Type> dict {get; private set;}
		protected SymbolTable prev;
		public Type parentType {get; private set;}
	
		public SymbolTable (SymbolTable parent = null, Type parentType = null)
		{
			dict = new Dictionary<string, Type> ();
			this.parent = parent;
			children = new List<SymbolTable> ();
			this.parentType = parentType;
		}

		public void put (string w, Type t)
		{
			dict.Add (w, t);
		}

		/*public Type getTypeForNode (IDParseNode node)
		{
			if (node is NumExprParseNode)
			{
				if (node.id.Contains ("."))
					return Type.Double;

				return Type.Integer;
			}

			return getType (node.id);
		}*/

		public Type getType (TypeNode node)
		{
			if (node.listNodes [0] is IDNode)
			{
				return getType (((IDNode)node.listNodes [0]).id);
			}
			else if (node is ArrayTypeNode)
			{
				Type type = null;
				int width = 0;
				foreach (ASTNode _node in node.listNodes)
				{
					if (_node is TypeNode)
						type = this.getType ((TypeNode)_node);
					else if (_node is NumExprNode)
						width = int.Parse (((NumExprNode)_node).value);
				}
				if (type == null || width == 0)
					return null;
				return new ArrayType (type, width);
			}
			else if (node.listNodes [0] is TokenNode)
			{
				TokenNode tokenNode = (TokenNode)node.listNodes[0];
				switch (tokenNode.token.tag)
				{
				case Tag.Int:
					return Type.Integer;
				case Tag.Short:
					return Type.Short;
				case Tag.Void:
					return Type.Void;
				case Tag.Double:
					return Type.Double;
				case Tag.Char:
					return Type.Character;
				case Tag.Float:
					return Type.Float;
				}
			}

			return null;
		}

		public Type getType (string w)
		{
			SymbolTable symTable = this;
			if (w.Contains ("."))
			{
				string s = w.Substring (0, w.IndexOf ("."));
				do
				{
					if (symTable.dict.ContainsKey (s))
					{
						//return ((StructType)symTable.dict [s]).symTable.getType (w.Substring (w.IndexOf (".") + 1));
					}

					symTable = symTable.parent;
				}
				while (symTable != null);
			}
			else
			{
				do
				{
					if (symTable.dict.ContainsKey (w))
						return symTable.dict [w];
						
					symTable = symTable.parent;
				}
				while (symTable != null);
			}

			return null;
		}

		public static void printSpaces (int level)
		{
			for (int k = 0; k < level*2; k++)
			{
				Console.Write (" ");
			}
		}

		public void print (int level = 0)
		{
			Dictionary <string, Type>.Enumerator e = dict.GetEnumerator ();
			do
			{
				printSpaces (level);
				Console.WriteLine (e.Current.Key + " " + e.Current.Value);
			}
			while (e.MoveNext ());
			foreach (SymbolTable symt in children)
				symt.print (level + 1);
		}

		public void addFromTable (SymbolTable table)
		{
			Dictionary <string, Type>.Enumerator e = table.dict.GetEnumerator ();
			do
			{
				if (e.Current.Key != null)
					dict.Add (e.Current.Key, e.Current.Value);
			}
			while (e.MoveNext ());
		}

		public int getTotalSize (bool includeFunctions = false)
		{
			int width = 0;
			Dictionary <string, Type>.Enumerator e = dict.GetEnumerator ();
			do
			{
				if (e.Current.Key == null)
					continue;

				if (includeFunctions && e.Current.Value is FunctionType)
					width += ((FunctionType)e.Current.Value).symTable.getTotalSize (true);
				else
					width += e.Current.Value.width;
			}
			while (e.MoveNext ());

			return width;
		}
	}
}
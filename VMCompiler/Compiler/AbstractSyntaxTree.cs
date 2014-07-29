using System;
using System.Collections.Generic;

namespace Compiler
{
	public abstract class ASTNode 
	{
		static int labels = 0;
		public List <ASTNode> listNodes;
		protected string currentLabel, endLabel;
		public static int tempCount = 0;

		protected ASTNode ()
		{
			listNodes = new List<ASTNode> ();
		}
		protected static string getTemporaryName ()
		{
			tempCount += 1;
			return "t" + tempCount;
		}

		public virtual void travelNode (int level = 0)
		{
			for (int i = 0; i < level * 2; i++)
				Console.Write (" ");
			Console.WriteLine (ToString ());
			if (listNodes != null)
			{
				//Console.WriteLine ("sdfsdfsdffs");
				foreach (ASTNode n in listNodes)
				{
					n.travelNode (level + 1);
				}
		    }
			else
			{
				Console.WriteLine ("LIST IS NULL");
			}
		}

		public virtual void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			foreach (ASTNode n in nodes)
			{
				if (n is TokenNode)
				{
					TokenNode tokenNode = ((TokenNode)n);
					if (tokenNode.token.tag == Tag.Comma || 
					    tokenNode.token.tag == Tag.EndBigBracket ||
					    tokenNode.token.tag == Tag.EndBrace ||
					    tokenNode.token.tag == Tag.StartBigBracket ||
					    tokenNode.token.tag == Tag.StartBrace ||
					    tokenNode.token.tag == Tag.StartParenthesis ||
					    tokenNode.token.tag == Tag.EndParenthesis ||
					    tokenNode.token.tag == Tag.EndOfLine ||
					    tokenNode.token.tag == Tag.If ||
					    tokenNode.token.tag == Tag.While ||
					    tokenNode.token.tag == Tag.Class ||
					    tokenNode.token.tag == Tag.Protected)
						continue;
				}
				listNodes.Add (n);
			}
		}

		public virtual string generateCode (SymbolTable symTable)
		{
			string s = "";
			if (listNodes != null)
			{
				foreach (ASTNode node in listNodes)
					s += node.generateCode (symTable);
			}

			return s;
		}

		public string generateLabel ()
		{
			labels += 1;
			return "L" + labels.ToString ();
		}
	}

	public class TokenNode : ASTNode
	{
		public Token token;
		public TokenNode (Token t)
		{
			token = t;
		}

		public override string ToString ()
		{
			return token.ToString ();
		}

		public override string generateCode (SymbolTable symTable)
		{
			return token.ToString ();
		}
	}

	public class ExpressionNode : ASTNode
	{
	}

	public class StatementNode : ASTNode
	{
	}

	public class ConstantNode : ExpressionNode
	{
	}

	public class ArgumentNode : ExpressionNode
	{
	}

	public class ArgumentListNode : ASTNode
	{
		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			listNodes = new List<ASTNode> ();
			foreach (ASTNode node in nodes)
			{
				if (node is ExpressionNode)
					listNodes.Add ((ExpressionNode)node);
				else if (node is ArgumentListNode)
				{
					foreach (ExpressionNode _node in ((ArgumentListNode)node).listNodes)
					{
						listNodes.Add (_node);
					}
				}
			}
		}
	}

	public class FunctionCall : ExpressionNode
	{
		public ExpressionNode functionToCall {get; private set;}
		public ArgumentListNode arglist {get; private set;}

		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			foreach (ASTNode node in nodes)
			{
				if (node is ExpressionNode)
				{
					functionToCall = (ExpressionNode)node;
				}
				else if (node is ArgumentListNode)
				{
					arglist = (ArgumentListNode)node;
				}
			}
		}
	}

	public class MemberAccessNode : ExpressionNode
	{
		public string objectName {get; private set;}
		public string memberName {get; private set;}

		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			objectName = ((IDNode)nodes[0]).id;
			memberName = ((IDNode)nodes[2]).id;
		}
	}

	public class IDNode : ExpressionNode
	{
		public string id{get; private set;}

		public IDNode (string _id)
		{
			id = _id;
		}

		public override void travelNode (int level = 0)
		{
			for (int k = 0; k < level * 2; k++)
				Console.Write (" ");
			Console.WriteLine ("ID ASTNode id = " + id);
		}

		public Type getType (SymbolTable symTable)
		{
			return symTable.getType (id);
		}

		public override string generateCode (SymbolTable symTable)
		{
			return id;
		}
	}

	public class NumExprNode : ConstantNode
	{
		public string value {get; private set;}

		public NumExprNode (string value)
		{
			this.value = value;
		}

		public override void travelNode (int level = 0)
		{
			for (int i = 0; i < level * 2; i++)
				Console.Write (" ");
			//if (castTo != null)
			//	Console.WriteLine ("NumExpr value = " + value + " castTo = " + castTo);
			//else
				Console.WriteLine ("NumExpr value = " + value);
		}

		public Compiler.Type getType (SymbolTable symTable)
		{
			if (value.Contains ("."))
				return Type.Double;
			return Type.Integer;
		}

		public override string generateCode (SymbolTable symTable)
		{
			return value;
		}
	}

	public class CharacterNode : ConstantNode
	{
		public char value {get; private set;}

		public CharacterNode (char value)
		{
			this.value = value;
		}

		public override void travelNode (int level = 0)
		{
			for (int i = 0; i < level * 2; i++)
				Console.Write (" ");
			//if (castTo != null)
			//	Console.WriteLine ("Character value = " + value + " castTo = " + castTo);
			//else
				Console.WriteLine ("Character value = " + value);
		}

		public Compiler.Type getType (SymbolTable symTable)
		{
			return Type.Character;
		}

		public override string generateCode (SymbolTable symTable)
		{
			return value.ToString ();
		}
	}

	public class StringNode : ConstantNode
	{
		public string value {get; private set;}

		public StringNode (string value)
		{
			this.value = value;
		}
	}

	public class ArrayIndexNode : ExpressionNode
	{
		public ExpressionNode index {get; private set;}
		public string arrayName {get; private set;}

		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			foreach (ASTNode node in nodes)
			{
				if (node is IDNode)
					arrayName = ((IDNode)node).id;
				else if (node is ExpressionNode)
					index = (ExpressionNode)node; 
			}
		}
	}

	public class CastExpressionNode : ExpressionNode
	{
		public Type castTo {get; private set;}
		public ExpressionNode expression {get; private set;}

		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			foreach (ASTNode node in nodes)
			{
				if (node is TypeNode)
					castTo = symTable.getType ((TypeNode)node);
				else if (node is ExpressionNode)
					expression = (ExpressionNode)node;
			}
		}
	}

	public class BinaryOperatorNode : ExpressionNode
	{
	}

	public class UnaryOperatorNode : ExpressionNode
	{
	}

	public class NewOperatorNode : ExpressionNode
	{
	}

	public class IfNode : StatementNode
	{
	}

	public class WhileNode : StatementNode
	{
	}

	public class ReturnNode : StatementNode
	{
	}

	public class Break : StatementNode
	{
	}

	public class CompoundStatementNode : StatementNode
	{
	}

	public class StatementListNode : StatementNode
	{
	}

	public class LocalDeclarationListNode : StatementNode
	{
		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			listNodes = new List<ASTNode> ();
			foreach (ASTNode node in nodes)
			{
				if (node is LocalDeclarationNode)
					listNodes.Add (node);
				else if (node is LocalDeclarationListNode)
				{
					foreach (ASTNode _node in ((LocalDeclarationListNode)node).listNodes)
					{
						if (_node is LocalDeclarationNode)
							listNodes.Add (_node);
					}
				}
			}
		}
	}

	public class ParameterNode : ASTNode
	{
		public string id;
		public Type type;
		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			foreach (ASTNode node in nodes)
			{
				if (node is IDNode)
					id = ((IDNode)node).id;
				else if (node is TypeNode)
				{
					type = symTable.getType ((TypeNode)node);
				}
			}
		}
	}

	public class ParameterListNode : ASTNode
	{
	}

	public class ClassDefinitionListNode : ASTNode
	{
	}

	public class ClassDefinitionNode : ASTNode
	{
		private IDNode classID;
		public MemberDeclarationList members;

		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			foreach (ASTNode node in nodes)
			{
				if (node is ClassSignatureNode)
					classID = ((ClassSignatureNode)node).classID;
				else if (node is MemberDeclarationList)
					members = (MemberDeclarationList)node;
			}
		}

		public string getClassName ()
		{
			return classID.id;
		}
	}

	public class ClassDefinitionWithParentNode : ClassDefinitionNode
	{
		public Type parent;
		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			base.addNodes (nodes, symTable);

			foreach (ASTNode node in nodes)
			{
				if (node is IDNode)
				{
					parent = symTable.getType (((IDNode)node).id);
				}
			}
		}
	}

	public class ArrayTypeNode : TypeNode
	{
	}

	public class LocalDeclarationNode : ASTNode
	{
		public string name {get; private set;}
		public Type type {get; private set;}
		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			foreach (ASTNode node in nodes)
			{
				if (node is IDNode)
					name = ((IDNode)node).id;
				else if (node is TypeNode)
				{
					type = symTable.getType ((TypeNode)node);
				}
			}
		}
	}

	public class ParameterArrayNode : ParameterNode
	{
	}

	public class ContinueNode : ASTNode
	{
	}

	public class ExpressionStatementNode : ASTNode
	{
	}

	public class BreakNode : ASTNode
	{
	}

	public class TypeNode : ASTNode
	{
	}

	public class MemberDeclaration : LocalDeclarationNode
	{
		public enum AccessSpecifier
		{
			Private,
			Protected,
			Public,
		}

		public AccessSpecifier accessSpecifier {get; private set;}
		public bool isStatic {get; private set;}

		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			base.addNodes (nodes, symTable);
			isStatic = false;
			foreach (ASTNode node in nodes)
			{
				if (node is TokenNode)
				{
					switch (((TokenNode)node).token.tag) 
					{
					case Tag.Public:
						accessSpecifier = AccessSpecifier.Public;
						break;
					case Tag.Protected:
						accessSpecifier = AccessSpecifier.Protected;
						break;
					case Tag.Private:
						accessSpecifier = AccessSpecifier.Private;
						break;
					case Tag.Static:
						isStatic = true;
						break;
					}
				}
			}
		}
	}

	public class MemberDeclarationList : ASTNode
	{
		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			listNodes = new List<ASTNode> ();
			foreach (ASTNode node in nodes)
			{
				if (node is MemberDeclaration)
					listNodes.Add (node);
				else if (node is MemberDeclarationList)
				{
					foreach (ASTNode _node in ((MemberDeclarationList)node).listNodes)
					{
						if (_node is MemberDeclaration)
							listNodes.Add (_node);
					}
				}
			}
		}		 
	}

	public class FunctionSignature : ASTNode
	{
	}

	public class FunctionDefinition : MemberDeclaration
	{
		public ParameterListNode paramList;
		public CompoundStatementNode compoundStatement;
		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			//Call For FunctionSignature
			base.addNodes (nodes[0].listNodes, symTable);
			foreach (ASTNode node in nodes[0].listNodes)
			{
				if (node is ParameterListNode)
				{
					paramList = (ParameterListNode)node;
					break;
				}
			}

			foreach (ASTNode node in nodes)
			{
				if (node is CompoundStatementNode)
					compoundStatement = (CompoundStatementNode)node;
			}
		}
	}

	public class FieldDeclaration : MemberDeclaration
	{
		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			base.addNodes (nodes, symTable);
		}
	}

	public class ClassSignatureNode : ASTNode
	{
		public IDNode classID{get; private set;}

		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			foreach (ASTNode node in nodes)
			{
				if (node is IDNode)
					classID = (IDNode)node;
			}
		}
	}
}
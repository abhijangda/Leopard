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

	public abstract class ExpressionNode : ASTNode
	{
		public abstract Compiler.Type getType (SymbolTable symTable);
	}

	public class StatementNode : ASTNode
	{
	}

	public abstract class ConstantNode : ExpressionNode
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

		public override Type getType (SymbolTable symTable)
		{
			return functionToCall.getType (symTable);
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

		public override Type getType (SymbolTable symTable)
		{
			return ((ClassType)symTable.getType (objectName)).symTable.getType (memberName);
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

		public override Type getType (SymbolTable symTable)
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

		public override Compiler.Type getType (SymbolTable symTable)
		{
			if (value.Contains ("."))
				return Type.Float;
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

		public override Compiler.Type getType (SymbolTable symTable)
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

		public override Type getType (SymbolTable symTable)
		{
			return null;
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

		public override Type getType (SymbolTable symTable)
		{
			return ((ArrayType)symTable.getType (arrayName)).type;
		}
	}

	public class CastExpressionNode : ExpressionNode
	{
		public string castToString {get; private set;}
		public Type castTo {get; private set;}
		public ExpressionNode expression {get; private set;}

		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			foreach (ASTNode node in nodes)
			{
				if (node is TypeNode)
				{
					castTo = symTable.getType ((TypeNode)node);
					castToString = ((IDNode)node.listNodes[0]).id;
				}
				else if (node is ExpressionNode)
				{
					expression = (ExpressionNode)node;
				}
			}
		}

		public override Type getType (SymbolTable symTable)
		{
			return castTo;
		}
	}

	public class BinaryOperatorNode : ExpressionNode
	{
		public Token op {get; private set;}
		public ExpressionNode expression1;
		public ExpressionNode expression2;

		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			foreach (ASTNode node in nodes)
			{
				if (node is TokenNode && ((TokenNode)node).token is Word)
				{
					op = ((Word)((TokenNode)node).token);
				}
				else if (node is ExpressionNode && !(expression1 is ExpressionNode))
				{
					expression1 = (ExpressionNode)node;
				}
				else if (node is ExpressionNode)
				{
					expression2 = (ExpressionNode)node;
				}
			}
		}

		public override Type getType (SymbolTable symTable)
		{
			return Type.max (expression1.getType (symTable), expression1.getType (symTable));
		}
	}

	public class UnaryOperatorNode : ExpressionNode
	{
		public Token op {get; private set;}
		public ExpressionNode expression;

		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			foreach (ASTNode node in nodes)
			{
				if (node is TokenNode && ((TokenNode)node).token is Word)
				{
					op = ((Word)((TokenNode)node).token);
				}
				else if (node is ExpressionNode)
				{
					expression = (ExpressionNode)node;
				}
			}
		}

		public override Type getType (SymbolTable symTable)
		{
			return expression.getType (symTable);
		}
	}

	public class NewOperatorNode : ExpressionNode
	{
		public Type newType {get; private set;}
		public FunctionCall constructorCall {get; private set;}
		public string newTypeName {get; private set;}

		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			foreach (ASTNode node in nodes)
			{
				if (node is FunctionCall)
				{
					constructorCall = (FunctionCall)node;
					if (constructorCall.functionToCall is IDNode)
					{
						newType = symTable.getType (((IDNode)constructorCall.functionToCall).id);
					}
					break;
				}
			}
		}

		public override Type getType (SymbolTable symTable)
		{
			return newType;
		}
	}

	public class IfNode : StatementNode
	{
		public StatementNode stmt{get; private set;}
		public ExpressionNode expression{get; private set;}
		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			stmt = null;
			expression = null;

			foreach (ASTNode node in nodes)
			{
				if (node is StatementNode)
					stmt = (StatementNode)node;
				else if (node is ExpressionNode)
					expression = (ExpressionNode)node;
			}
		}
	}

	public class WhileNode : StatementNode
	{
		public StatementNode stmt{get; private set;}
		public ExpressionNode expression{get; private set;}
		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			stmt = null;
			expression = null;

			foreach (ASTNode node in nodes)
			{
				if (node is StatementNode)
					stmt = (StatementNode)node;
				else if (node is ExpressionNode)
					expression = (ExpressionNode)node;
			}
		}
	}

	public class ForNode : StatementNode
	{
		public StatementNode stmt{get; private set;}
		public ExpressionNode initExpression{get; private set;}
		public ExpressionNode condition{get; private set;}
		public ExpressionNode increment{get; private set;}

		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			stmt = null;
			initExpression = null;
			condition = null;
			increment = null;

			foreach (ASTNode node in nodes)
			{
				if (node is StatementNode)
					stmt = (StatementNode)node;
				else if (node is ExpressionNode)
				{
					if (initExpression == null)
						initExpression = (ExpressionNode)node;
					else if (condition == null)
						condition = (ExpressionNode)node;
					else if (increment == null)
						increment = (ExpressionNode)node;
				}
			}
		}
	}
	public class ReturnNode : StatementNode
	{
		public ExpressionNode expression {get; private set;}

		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			expression = null;

			foreach (ASTNode node in nodes)
			{
				if (node is ExpressionNode)
				{
					expression = (ExpressionNode)node;
					break;
				}
			}
		}
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
		public IDNode parent {get; private set;}
		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			base.addNodes (nodes, symTable);

			foreach (ASTNode node in nodes)
			{
				if (node is ClassSignatureWithParentNode)
				{
					parent = ((ClassSignatureWithParentNode)node).parentID;
					break;
				}
			}
		}
	}

	public class ArrayTypeNode : TypeNode
	{
	}

	public class LocalDeclarationNode : ASTNode
	{
		public string name {get; protected set;}
		public Type type {get; protected set;}
		public string typeName {get; protected set;}

		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			foreach (ASTNode node in nodes)
			{
				if (node is IDNode)
					name = ((IDNode)node).id;
				else if (node is TypeNode)
				{
					type = symTable.getType ((TypeNode)node);
					typeName = node.ToString ();
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
		public AccessSpecifier accessSpecifier {get; protected set;}
		public bool isStatic {get; protected set;}

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
			type = new MemberType (accessSpecifier, type, isStatic, (ClassType)symTable.parentType);
		}
	}

	public class ConstructorDefinition : FunctionDefinition
	{
		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			isStatic = false;

			foreach (ASTNode node in nodes[0].listNodes)
			{
				if (node is IDNode)
				{
					name = ((IDNode)node).id;
					type = symTable.getClassType (name);
				}
				else if (node is TokenNode)
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

			foreach (ASTNode node in nodes)
			{
				if (node is ParameterListNode)
				{
					paramList = (ParameterListNode)node;
				}
				else if (node is CompoundStatementNode)
				{
					compoundStatement = (CompoundStatementNode)node;
				}
			}

			type = new ConstructorType (accessSpecifier, type, paramList, symTable, (ClassType)symTable.parent.parentType);
		}
	}

	public class ConstructorCall : FunctionCall
	{
	}

	public class ConstructorSignature : FunctionSignature
	{
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
				}
				else if (node is CompoundStatementNode)
				{
					compoundStatement = (CompoundStatementNode)node;
				}
			}

			type = new FunctionType (((MemberType)type).accessSpecifier, ((MemberType)type).isStatic, 
			                         ((MemberType)type).type, symTable, paramList, (ClassType)symTable.parent.parentType);
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
		public IDNode classID{get; protected set;}

		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			foreach (ASTNode node in nodes)
			{
				if (node is IDNode)
					classID = (IDNode)node;
			}
		}
	}

	public class ClassSignatureWithParentNode : ClassSignatureNode
	{
		public IDNode parentID {get; private set;}

		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			classID = null;
			parentID = null;
			foreach (ASTNode node in nodes)
			{
				if (node is IDNode)
				{
					if (classID == null)
						classID = (IDNode)node;
					else if (parentID == null)
						parentID = (IDNode)node;
				}
			}
		}
	}
}
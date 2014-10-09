using System;
using System.Collections.Generic;

namespace Compiler
{
	public class Code
	{
		public string code {get; private set;}
		public string returnExpression {get; private set;}

		public Code (string code, string returnExpression)
		{
			this.code = code;
			this.returnExpression = returnExpression;
		}
	}

	public abstract class ASTNode
	{
		static int labels = 0;
		public List <ASTNode> listNodes;
		protected string currentLabel, endLabel;
		public static int tempCount = 0;
		public static SymbolTable tempsSymTable;

		static ASTNode()
		{
			tempsSymTable = new SymbolTable ();
		}

		protected ASTNode ()
		{
			listNodes = new List<ASTNode> ();
		}

		protected static string getTemporaryName (ExpressionNode expression, SymbolTable symTable)
		{
			tempCount += 1;
			string temp = "t" + tempCount;
			tempsSymTable.put (temp, new TemporaryType (expression.getType (symTable)));
			return temp;
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

		public virtual Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			string s = "";
			if (listNodes != null)
			{
				foreach (ASTNode node in listNodes)
					s += node.generateCode (currSymTable, rootSymTable, indent + 1).code;
			}

			return new Code (s, "");
		}

		public string generateLabel ()
		{
			labels += 1;
			return "L" + labels.ToString ();
		}

		public static string getIndentString (int indent)
		{
			string s = "";
			for (int i = 0; i < indent; i++)
				s += " ";
			return s;
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

		public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			return new Code (token.ToString (), "");
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

		public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			Code functoCallCode = functionToCall.generateCode (currSymTable, rootSymTable, 0);
			string s = "";
			if (functoCallCode.code != "")
			{
				s += functoCallCode.code + "\n";
			}

			string callCode = "";
			if (arglist != null && arglist.listNodes != null)
			{
				for (int i = 0; i < arglist.listNodes.Count; i++)
				{
					ASTNode node = arglist.listNodes[i];
					Code argCode = node.generateCode (currSymTable, rootSymTable, indent);
					s += argCode.code + "\n";
					callCode += getIndentString (indent) + "param " + argCode.returnExpression + "\n";
				}
			}
			string temp = getTemporaryName (functionToCall, currSymTable);
			Type type = getType (currSymTable);
			if ((type is FunctionType && ((FunctionType)type).returnType != Type.Void) || (type is ConstructorType))
				callCode += getIndentString (indent) + temp + " = call " + functoCallCode.returnExpression;
			else
				callCode += getIndentString (indent) + "call " + functoCallCode.returnExpression;

			return new Code (s + callCode, temp);
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

		public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			string temp = getTemporaryName (this, currSymTable);
			return new Code ("", objectName + "." + memberName);
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

		public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			return new Code ("", id);
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

		public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			return new Code ("", value);
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

		public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			return new Code ("", "'" + value.ToString () + "'");
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

		public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			return new Code ("", "\"" + value + "\"");
		}
	}

	public class ArrayIndexNode : ExpressionNode
	{
		public ExpressionNode index {get; private set;}
		public string arrayName {get; private set;}
		public bool isfirst;

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

		public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			string temp = getTemporaryName (this, currSymTable);
			Code indexCode = index.generateCode (currSymTable, rootSymTable, indent);
			if (isfirst)
				return new Code (indexCode.code + "\n" + getIndentString (indent) + 
					arrayName + "[" + indexCode.returnExpression + "] = " + temp, temp);
			else
				return new Code (indexCode.code + "\n" + getIndentString (indent) + 
					temp + " = " + arrayName + "[" + indexCode.returnExpression + "]", temp);
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

		public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			string temp = getTemporaryName (this, rootSymTable);
			Code code = expression.generateCode (currSymTable, rootSymTable, indent);
			return new Code (code.code + "\n" + getIndentString (indent) + temp + " = cast (" + castTo + ") " + code.returnExpression, temp);
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
			Type t1 = expression1.getType (symTable);
			Type t2 = expression1.getType (symTable);
			return Type.max (t1, t2);
		}

		public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			if (expression1 is ArrayIndexNode)
			{
				((ArrayIndexNode)expression1).isfirst = true && op == Word.Equal;
			}

			if (expression2 is ArrayIndexNode)
			{
				((ArrayIndexNode)expression2).isfirst = false && op == Word.Equal;
			}

			Code expressionCode1 = expression1.generateCode (currSymTable, rootSymTable, indent);
			Code expressionCode2 = expression2.generateCode (currSymTable, rootSymTable, indent);
			string s = "";
	
			if (op != Word.Equal && expressionCode1.code != "")
				s += expressionCode1.code + "\n";

			if (expressionCode2.code != "")
				s += expressionCode2.code + "\n";

			string temp = "";
			if (op != Word.Equal)
			{
				temp = getTemporaryName (this, currSymTable);
				s += getIndentString (indent) + temp + " = " + expressionCode1.returnExpression + " " + op.ToString () + " " + expressionCode2.returnExpression;
			}
			else
			{
				s += getIndentString (indent) + expressionCode1.returnExpression + " = " + expressionCode2.returnExpression;
				if (op == Word.Equal && expression1 is ArrayIndexNode && expressionCode1.code != "")
					s += expressionCode1.code + "\n";
			}

			return new Code (s, temp);
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

	public class ArrayConstructorCall : ExpressionNode
	{
		public int size {get; private set;}
		public Type type {get; private set;}
		public string name {get; private set;}
	
		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			foreach (ASTNode node in nodes)
			{
				if (node is NumExprNode)
				{
					size = int.Parse (((NumExprNode)node).value);
				}
			    else if (node is TypeNode)
				{
					type = symTable.getType ((TypeNode)node);
				}
			}
		}

		public override Type getType (SymbolTable symTable)
		{
			return new ArrayType (type, size);
		}

		public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			string temp = getTemporaryName (this, currSymTable);
			return new Code ("", temp);
		}
	}

	public class NewOperatorNode : ExpressionNode
	{
		public Type newType {get; private set;}
		public ExpressionNode constructorCall {get; private set;}
		public string newTypeName {get; private set;}

		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			foreach (ASTNode node in nodes)
			{
				if (node is ArrayConstructorCall)
				{
					newType = ((ArrayConstructorCall)node).type;
					newTypeName = ((ArrayConstructorCall)node).type + "[" + ((ArrayConstructorCall)node).size + "]";
					constructorCall = (ExpressionNode)node;

					break;
				}

				if (node is ConstructorCall)
				{
					constructorCall = (ExpressionNode)node;
					ConstructorCall _node = (ConstructorCall)node;
					if (_node.functionToCall is IDNode)
					{
						newType = symTable.getType (((IDNode)_node.functionToCall).id);
						newTypeName = ((IDNode)_node.functionToCall).id;
					}
					break;
				}
			}
		}

		public override Type getType (SymbolTable symTable)
		{
			return newType;
		}

		public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			Code constructorCode = constructorCall.generateCode (currSymTable, rootSymTable, indent);
			return new Code (getIndentString (indent) + constructorCode.returnExpression + " = new " + newTypeName + "\n",
				constructorCode.returnExpression);
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

		public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			Code expressionCode = expression.generateCode (currSymTable, rootSymTable, indent);
			Code statementCode = stmt.generateCode (currSymTable, rootSymTable, indent);

			currentLabel = generateLabel ();

			string s = "";
			s = getIndentString (indent) + expressionCode.code + "\n";
			s += getIndentString (indent+1) + "iffalse " + expressionCode.returnExpression + " ";
			s += "goto " + currentLabel + "\n";
			s += getIndentString (indent) + statementCode.code + "\n";
			s += getIndentString (indent+1) + currentLabel + ":\n";
			
			return new Code (s, "");
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

		public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			Code expressionCode = expression.generateCode (currSymTable, rootSymTable, indent);
			Code statementCode = stmt.generateCode (currSymTable, rootSymTable, indent);

			currentLabel = generateLabel ();
			endLabel = generateLabel ();

			string s = getIndentString (indent) + currentLabel + ":\n";
			s += getIndentString (indent) + expressionCode.code + "\n";
			s += getIndentString (indent) + "iffalse " + expressionCode.returnExpression + " ";
			s += "goto " + endLabel + "\n";
			s += getIndentString (indent) + statementCode.code + "\n";
			s += getIndentString (indent) + endLabel + ":\n";

			return new Code (s, "");
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

		public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			Code initCode = initExpression.generateCode (currSymTable, rootSymTable, indent);
			string s = initCode.code + "\n";
			currentLabel = generateLabel ();
			s += getIndentString (indent) + currentLabel + ":\n";
			endLabel = generateLabel ();
			Code condCode = condition.generateCode (currSymTable, rootSymTable, indent);
			s += stmt.generateCode (currSymTable, rootSymTable, indent).code + "\n";
			s += increment.generateCode (currSymTable, rootSymTable, indent).code + "\n";
			s += condCode.code + "\n";
			s += getIndentString (indent) + "iffalse " + condCode.returnExpression + " ";
			s += "goto " + endLabel + "\n";
			s += getIndentString (indent) + "goto " + currentLabel + "\n";
			s += getIndentString (indent) + endLabel + ":\n";

			return new Code (s, "");
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

		public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			if (expression != null)
			{
				Code code = expression.generateCode (currSymTable, rootSymTable, 0);
				return new Code (code.code + "\n" + getIndentString (indent) + "return " + code.returnExpression, "");
			}

			return new Code (getIndentString (indent) + "return", "");
		}
	}

	public class CompoundStatementNode : StatementNode
	{
		public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			string s;
			if (listNodes.Count == 0)
			{
				return new Code ("", "");
			}
			s = "";
			foreach (ASTNode node in listNodes)
			{
				s += node.generateCode (currSymTable, rootSymTable, indent + 1).code;
			}

			return new Code (s, "");
		}
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

		public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			return new Code (type + " " + id, "");
		}
	}

	public class ParameterListNode : ASTNode
	{
		public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			string s = "(";
			foreach (ASTNode node in listNodes)
			{
				if (node != listNodes[listNodes.Count - 1])
				{
					s += node.generateCode (currSymTable, rootSymTable, 0).code + ", ";
				}
				else
				{
					s += node.generateCode (currSymTable, rootSymTable, 0).code;
				}
			}

			s += ")";

			return new Code (s, "");
		}
	}

	public class ClassDefinitionListNode : ASTNode
	{
	}

	public class ClassDefinitionNode : ASTNode
	{
		protected IDNode classID;
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

		public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			string s = "";

			s += ".class " + classID.id + " extends None \n{\n";

			SymbolTable classSymTable = ((ClassType)currSymTable.getClassType (classID.id)).symTable;

			foreach (ASTNode node in members.listNodes)
			{
				if (node is FieldDeclaration)
					s += node.generateCode (classSymTable, rootSymTable, indent + 1).code + "\n";
			}

			foreach (ASTNode node in members.listNodes)
			{
				if (!(node is FieldDeclaration))
					s += node.generateCode (classSymTable, rootSymTable, indent + 1).code + "\n";
			}

			s += "}\n";
		
			return new Code (s, "");
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

		public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			string s = "";

			s += ".class " + classID.id + " extends " + parent.id +"\n{\n\t";

			SymbolTable classSymTable = ((ClassType)currSymTable.getClassType (classID.id)).symTable;

			foreach (ASTNode node in members.listNodes)
			{
				s += "\t." + node.generateCode (classSymTable, rootSymTable, indent + 1) + "\n";
			}

			s += "}\n";

			return new Code (s, "");
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
			while (symTable.parent != null)
				symTable = symTable.parent;

			foreach (ASTNode node in nodes)
			{
				if (node is IDNode)
					name = ((IDNode)node).id;
				else if (node is TypeNode)
				{
					type = symTable.getType ((TypeNode)node);
					typeName = type.ToString ();
				}
			}
		}
	}

	public class ArrayDeclarationNode : LocalDeclarationNode
	{
		public int width {get; private set;}
		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			foreach (ASTNode node in nodes)
			{
				if (node is IDNode)
				{
					name = ((IDNode)node).id;
				}
				else if (node is NumExprNode)
				{
					width = int.Parse (((NumExprNode)node).value);
				}
			}

			foreach (ASTNode node in nodes)
			{
				if (node is TypeNode)
				{
					type = new ArrayType (symTable.getType ((TypeNode)node), width);
				}
			}
		}
	}

	public class ParameterArrayNode : ParameterNode
	{
	}

	public class ContinueNode : ASTNode
	{
		public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			return new Code ("goto " + currentLabel, "");
		}
	}

	public class ExpressionStatementNode : ASTNode
	{
		public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			string s = "";
			foreach (ASTNode node in listNodes)
			{
				s += node.generateCode (currSymTable, rootSymTable, indent).code + "\n";
			}

			return new Code (s, "");
		}
	}

	public class BreakNode : StatementNode
	{
		public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			return new Code (getIndentString (indent) + "goto " + endLabel, "");
		}
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

		public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			string s = "";

			s = accessSpecifier.ToString ();

			if (isStatic)
				s += " static";
			s += " " + typeName + " " + name;;

			return new Code (getIndentString (indent) + s, "");
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
				else if (node is ParameterListNode)
				{
					paramList = (ParameterListNode)node;
				}
			}

			foreach (ASTNode node in nodes)
			{
				if (node is CompoundStatementNode)
				{
					compoundStatement = (CompoundStatementNode)node;
				}
			}

			type = new ConstructorType (accessSpecifier, type, paramList, symTable, (ClassType)symTable.parent.parentType);
		}

		/*public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			string s = getIndentString (indent);
			if (paramList != null)
			{
				s += ".ctor " + base.generateCode (currSymTable, rootSymTable, 0).code + " " + 
					paramList.generateCode (currSymTable, rootSymTable, 0).code + "\n";
			}
			else
			{
				s += ".ctor " + base.generateCode (currSymTable, rootSymTable, 0).code + "\n";
			}
			if (compoundStatement != null)
			{
				s += compoundStatement.generateCode (((ConstructorType)currSymTable.getType (name)).symTable,
					rootSymTable, indent).code;
			}
			else
			{
				s += getIndentString (indent) + "{\n" + getIndentString (indent) +"}\n";
			}
			return new Code (s, "");
		}*/
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
					break;
				}
			}
			foreach (ASTNode node in nodes)
			{
				if (node is CompoundStatementNode)
				{
					compoundStatement = (CompoundStatementNode)node;
				}
			}

			type = new FunctionType (((MemberType)type).accessSpecifier, ((MemberType)type).isStatic, 
			                         ((MemberType)type).type, symTable, paramList, (ClassType)symTable.parent.parentType);
		}

		public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			string s = getIndentString (indent);
			string toadd = ".method ";
			if (this is ConstructorDefinition)
				toadd = ".ctor ";
	
			if (paramList != null)
			{
				s += toadd + base.generateCode (currSymTable, rootSymTable, 0).code + " " + 
					paramList.generateCode (currSymTable, rootSymTable, 0).code + "\n";
			}
			else
			{
				s += toadd + base.generateCode (currSymTable, rootSymTable, 0).code + " ()\n";
			}
			s += getIndentString (indent) + "{\n";
			if (compoundStatement != null)
			{
				if (this is ConstructorDefinition)
				{
					s += compoundStatement.generateCode (((ConstructorType)currSymTable.getType (name)).symTable,
						rootSymTable, indent).code;
				}
				else
				{
					s += compoundStatement.generateCode (((FunctionType)currSymTable.getType (name)).symTable,
						rootSymTable, indent).code;
				}
			}
			s += getIndentString (indent) +"}\n";

			return new Code (s, "");
		}
	}

	public class FieldDeclaration : MemberDeclaration
	{
		public override void addNodes (List<ASTNode> nodes, SymbolTable symTable)
		{
			base.addNodes (nodes, symTable);
		}

		public override Code generateCode (SymbolTable currSymTable, SymbolTable rootSymTable, int indent)
		{
			 
			return new Code (getIndentString (indent) + ".field " + base.generateCode (currSymTable, rootSymTable, 0).code, "");
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
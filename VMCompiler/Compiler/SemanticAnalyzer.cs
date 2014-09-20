using System;
using System.Collections.Generic;

namespace Compiler
{
	public class SemanticAnalyzer
	{
		private static bool analyze_expression (ExpressionNode node, SymbolTable symTableRoot, 
												SymbolTable currentSymTable, int linenumber, 
												out CompileError error, out Type t)
		{
			error = null;
			t = node.getType (currentSymTable);
	
			if (t is MemberType)
			{
				if (((MemberType)t).accessSpecifier == AccessSpecifier.Private &&
				    ((MemberType)t).parentType != currentSymTable.parent.parentType)
				{
					error = new CompileError (node.ToString () + " is declared private", 
					                          linenumber);
					return false;
				}
			}

			return true;
		}

		public static bool analyze (ASTNode node, SymbolTable symTableRoot, 
		                            SymbolTable currentSymTable, int linenumber, 
		                            out CompileError error)
		{
			error = null;
			if (node is ClassDefinitionWithParentNode)
			{
				if (!(symTableRoot.getClassType (((ClassDefinitionWithParentNode)node).parent.id) != null))
				{
					error = new CompileError ("Cannot find class '" + ((ClassDefinitionWithParentNode)node).parent.id + "'", 
					                          linenumber);
					return false;
				}
			}
			else if (node is IDNode)
			{
				Type t = currentSymTable.getType (((IDNode)node).id);
				if (t == null)
				{
					error = new CompileError ("No type '" + ((IDNode)node).id + "' defined", 
					                          linenumber);
					return false;
				}
				if (t is MemberType)
				{
					if (((MemberType)t).accessSpecifier == AccessSpecifier.Private &&
					    ((MemberType)t).parentType != currentSymTable.parent.parentType)
					{
						error = new CompileError (((IDNode)node).id + "is declared private", 
						                          linenumber);
						return false;
					}
				}
			}
			else if (node is CastExpressionNode)
			{
				CastExpressionNode expr = (CastExpressionNode)node;

				if (expr.castTo == null)
				{
					error = new CompileError ("No type '" + expr.castToString + "' defined", 
					                          linenumber);
					return false;
				}

				Type t;
				if (!analyze_expression (expr.expression, symTableRoot, currentSymTable,
					linenumber, out error, out t))
					return false;
			}
			else if (node is BinaryOperatorNode)
			{
				BinaryOperatorNode binOpNode = (BinaryOperatorNode)node;

				Type t1;
				Type t2;

				if (!analyze_expression (binOpNode.expression1, symTableRoot, currentSymTable,
					linenumber, out error, out t1))
					return false;

				if (!analyze_expression (binOpNode.expression1, symTableRoot, currentSymTable,
					linenumber, out error, out t2))
					return false;

				if (t1 != t2)
				{
					error = new CompileError ("Types of two expressions are not equal", 
					                          linenumber);
					return false;
				}
			}
			else if (node is UnaryOperatorNode)
			{
				Type t;
				UnaryOperatorNode expr = (UnaryOperatorNode)node;
				if (!analyze_expression (expr.expression, symTableRoot, currentSymTable,
					linenumber, out error, out t))
					return false;
				if (!Type.numeric (t))
				{
					error = new CompileError ("Type must be numeric", 
					                          linenumber);
					return false;
				}
			}
			else if (node is ConstructorCall)
			{
				ConstructorCall newNode = (ConstructorCall)node;
			}
			else if (node is FunctionCall)
			{
				FunctionCall newNode = (FunctionCall)node;
				Type type = newNode.functionToCall.getType (currentSymTable);
				if (!(type is FunctionType))
				{
					error = new CompileError ("No function named '" + newNode.functionToCall.ToString () + "' defined",
					                          linenumber);
					return false;
				}

				if (newNode.arglist.listNodes.Count !=
				    ((FunctionType)type).listParamTypes.Count)
				{
					error = new CompileError ("No of formal paramters is not equal to actual parameters", 
					                          linenumber);
					return false;
				}

				for (int i = 0; i < newNode.arglist.listNodes.Count; i++)
				{
					if (((ExpressionNode)newNode.arglist.listNodes[i]).getType (currentSymTable) !=
					    ((FunctionType)type).listParamTypes [i])
					{
						error = new CompileError ("Type mismatch at parameter " + i, 
						                          linenumber);
						return false;
					}
				}
			}
			else if (node is MemberAccessNode)
			{
				MemberAccessNode memNode = (MemberAccessNode)node;

				ClassType type = (ClassType)currentSymTable.getType (memNode.objectName);
				MemberType memType = (MemberType)type.symTable.getType (memNode.memberName);
				if (memType == null)
				{
					error = new CompileError ("No member named '" + memNode.memberName + "' in '" + type.name + "'", 
					                          linenumber);
					return false;
				}

				if (memType.accessSpecifier != AccessSpecifier.Public)
				{
					error = new CompileError ("Member '" + memNode.memberName + "' of '" + type.name + "' is not Public", 
					                          linenumber);
					return false;
				}
			}
			else if (node is CastExpressionNode)
			{
				CastExpressionNode cast = (CastExpressionNode)node;
				if (cast.castTo == null)
				{
					error = new CompileError ("No type '" + cast.castToString + "' defined", 
					                          linenumber);
					return false;
				}
			}
			else if (node is NewOperatorNode)
			{
				NewOperatorNode newNode = (NewOperatorNode)node;

				if (newNode.newType == null)
				{
					error = new CompileError ("No type '" + newNode.newTypeName + "' defined", 
					                          linenumber);
					return false;
				}

				Type type = ((ClassType)newNode.newType).symTable.getType (((ClassType)newNode.newType).name);

				if (!(type is ConstructorType))
				{
					error = new CompileError ("No constructor in class '" + newNode.newTypeName + "' defined", 
					                          linenumber);
					return false;
				}

				if (newNode.constructorCall.arglist.listNodes.Count != 
				    ((ConstructorType)type).listParamTypes.Count)
				{
					error = new CompileError ("No of formal paramters is not equal to actual parameters", 
					                          linenumber);
					return false;
				}

				for (int i = 0; i < newNode.constructorCall.arglist.listNodes.Count; i++)
				{
					if (((ExpressionNode)newNode.constructorCall.arglist.listNodes[i]).getType (currentSymTable) !=
					    ((ConstructorType)type).listParamTypes [i])
					{
						error = new CompileError ("Type mismatch at parameter " + i, 
						                          linenumber);
						return false;
					}
				}
			}
			else if (node is LocalDeclarationNode)
			{
				LocalDeclarationNode declNode = (LocalDeclarationNode)node;
				if (declNode.type == null)
				{
					error = new CompileError ("Type " + declNode.typeName + " not defined", 
					                          linenumber);
					return false;
				}
			}
			return false;
		}
	}
}


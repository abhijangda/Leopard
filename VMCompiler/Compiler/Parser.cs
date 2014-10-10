using System;
using System.IO;
using LanguageGrammar;
using System.Collections.Generic;
/****
 * Work on Array *
 * 
 * 
 *****/

namespace Compiler
{
	public class Parser
	{
		Lexer lexer;
		Grammar g;
		Stack <int> stackStates;
		public SymbolTable symTableTree;
		List<CompileError> errors;

		public Parser (string file)
		{
			string grammar = File.ReadAllText ("./../../language");
			g = new Grammar (grammar);
			g.items ();
			g.createActionTable ();
			g.createGoToTable ();

			//g.displayStates ();
			lexer = new Lexer (file);
			stackStates = new Stack<int> ();
			stackStates.Push (0);

			symTableTree = new SymbolTable (null);
			errors = new List<CompileError> ();
		}

		private ASTNode getNode (Production p, SymbolTable symTable)
		{
			if (p.head.ToString () == "local-array-declaration" &&
				p.body.Count > 1)
				return new ArrayDeclarationNode ();
			if (p.head.ToString () == "type-specifier")
				return new TypeNode ();
			if (p.head.ToString () == "arg-list")
				return new ArgumentListNode ();
			if (p.head.ToString () == "member-declaration-list")
				return new MemberDeclarationList ();
			if (p.head.ToString () == "param-list")
				return new ParameterListNode ();
			if (p.body.Count == 1 && !(p.body [0] is Terminal))
				return null;
			if (p.head.ToString () == "constructor-definition")
				return new ConstructorDefinition ();
			if (p.head.ToString () == "constructor-signature")
				return new ConstructorSignature ();
			if (p.head.ToString () == "cast-expression")
				return new CastExpressionNode ();
			if (p.head.ToString () == "param-type-list")
				return new ParameterNode ();
			if (p.head.ToString () == "class-signature")
				return new ClassSignatureNode ();
			if (p.head.ToString () == "class-signature-with-parent")
				return new ClassSignatureWithParentNode ();
			if (p.head.ToString () == "class-with-parent-definition")
				return new ClassDefinitionWithParentNode ();
			if (p.head.ToString () == "new-expression")
				return new NewOperatorNode ();
			if (p.head.ToString () == "member-access")
				return new MemberAccessNode ();
			if (p.head.ToString () == "class-definition")
				return new ClassDefinitionNode ();
			if (p.head.ToString () == "param-id" && 
			    p.body.Count == 3)
				return new ParameterArrayNode ();
			if (p.head.ToString () == "local-declaration")
				return new LocalDeclarationNode ();
			if (p.head.ToString () == "class-definition-list")
				return new ClassDefinitionListNode ();
			if (p.head.ToString () == "fun-definition")
				return new FunctionDefinition ();
			if (p.head.ToString () == "fun-signature")
				return new FunctionSignature ();
			if (p.head.ToString () == "compound-stmt")
				return new CompoundStatementNode ();
			if (p.head.ToString () == "field-declaration")
				return new FieldDeclaration ();
			if (p.head.ToString () == "local-declarations")
				return new LocalDeclarationListNode ();
			if (p.head.ToString () == "statement-list")
				return new StatementListNode ();
			if (p.head.ToString () == "selection-stmt")
				return new IfNode ();
			if (p.head.ToString () == "iteration-stmt")
			{
				if (p.body.Count == 5)
					return new WhileNode ();
				return new ForNode ();
			}
			if (p.head.ToString () == "return-stmt")
				return new ReturnNode ();
			if (p.head.ToString () == "break-stmt")
				return new BreakNode ();
			if (p.head.ToString () == "continue-stmt")
				return new ContinueNode ();
			if (p.head.ToString () == "expression-stmt")
				return new ExpressionStatementNode ();
			if (p.head.ToString () == "expression" ||
			    p.head.ToString () == "term" ||
			    p.head.ToString () == "simple-expression" ||
			    p.head.ToString () == "and-expression" ||
			    p.head.ToString () == "sum-expression" ||
			    p.head.ToString () == "unary-expression" ||
			    p.head.ToString () == "immutable" ||
			    p.head.ToString () == "rel-expression" ||
			    p.head.ToString () == "logical-and-expression" ||
			    p.head.ToString () == "incor-expression" ||
			    p.head.ToString () == "exor-expression")
				return new BinaryOperatorNode ();

			if (p.head.ToString () == "unary-rel-expression")
				return new UnaryOperatorNode ();
			if (p.head.ToString () == "call")
				return new FunctionCall ();
			if (p.head.ToString () == "constructor-call")
				return new ConstructorCall ();
			if (p.head.ToString () == "array-constructor-call")
				return new ArrayConstructorCall ();
			if (p.head.ToString () == "mutable" && 
				p.body.Count == 4)
				return new ArrayIndexNode ();

			return null;
		}

		public ASTNode startParsing ()
		{
			Token t = lexer.scan ();
			string sym_str = "";
			sym_str = t.ToString ();
			SymbolTable currentSymTable = symTableTree;
			Stack <ASTNode> nodeStack = new Stack<ASTNode> ();

			if (t!= null && t.tag == Tag.ID)
			{
				sym_str = "ID";
				nodeStack.Push (new IDNode (t.ToString ()));
			}
			else if (t != null && t.tag == Tag.Character)
			{
				sym_str = "character";
				nodeStack.Push (new CharacterNode (t.ToString () [0]));
			}
			else if (t != null && t.tag == Tag.Number)
			{
				sym_str = "num";
				nodeStack.Push (new NumExprNode (t.ToString ()));
			}

			else if (t!= null)
			{
				sym_str = t.ToString ();
				nodeStack.Push (new TokenNode (t));
			}

			while (true)
			{
				ActionTable.Action action = null;	
				action = g.actionTable.getAction (sym_str, stackStates.Peek ());

				if (action == null)
				{
					if (t!= null)
						Console.WriteLine ("Error with action " + sym_str + " " + t.ToString () + " " + stackStates.Peek ());
					else
						Console.WriteLine ("Error with action " + sym_str + " " + stackStates.Peek ());

					return null;
				}

				if (action.type == ActionTable.Action.ActionType.Shift)
				{
					stackStates.Push (action.toState);
					t = lexer.scan ();

					if (t!= null && t.tag == Tag.ID)
					{
						sym_str = "ID";
						Console.WriteLine ("ID is " + t.ToString ());
						nodeStack.Push (new IDNode (t.ToString ()));
					}
					else if (t != null && t.tag == Tag.Character)
					{
						sym_str = "character";
						nodeStack.Push (new CharacterNode (t.ToString () [0]));
					}
					else if (t != null && t.tag == Tag.String)
					{
						sym_str = "string";
						nodeStack.Push (new StringNode (t.ToString ()));
					}
					else if (t != null && (t.tag == Tag.Number || t.tag == Tag.Real))
					{
						sym_str = "num";
						nodeStack.Push (new NumExprNode (t.ToString ()));
					}

					else if (t!= null)
					{
						sym_str = t.ToString ();
						if (t.ToString () != "$")
							nodeStack.Push (new TokenNode (t));
					}
				}

				else if (action.type == ActionTable.Action.ActionType.Reduce)
				{
					Production p = action.reduceTo;
					List<ASTNode> nodesToReduce = new List<ASTNode> ();
					ASTNode newNode = getNode (p, currentSymTable);
					ASTNode topNode = null;

					if (sym_str != "$")
				    	 topNode = nodeStack.Pop ();

					for (int i = 0; i < p.body.Count; i++)
					{
						stackStates.Pop ();
					    if (newNode != null)
						{
							ASTNode n = nodeStack.Pop ();
							Console.WriteLine ("popping " + n.ToString ());
							nodesToReduce.Add (n);
						}
					}

					if (nodesToReduce.Count != 0)
					{
						nodesToReduce.Reverse ();
						newNode.addNodes (nodesToReduce, currentSymTable);
						nodeStack.Push (newNode);
						//CompileError error;
						if (newNode is ConstructorDefinition)
						{
							currentSymTable = currentSymTable.parent;
							currentSymTable.dict.Add (((ConstructorDefinition)newNode).name,
							                          ((ConstructorDefinition)newNode).type);
						}
						else if (newNode is FunctionDefinition)
						{
							//When definition of function is obtained load the parent symbol table
							currentSymTable = currentSymTable.parent;
							currentSymTable.dict.Add (((FunctionDefinition)newNode).name,
							                          ((FunctionDefinition)newNode).type);
						}
						else if (newNode is LocalDeclarationNode)
						{
							currentSymTable.dict.Add (((LocalDeclarationNode)newNode).name,
							                          ((LocalDeclarationNode)newNode).type);
						}
						else if (newNode is FunctionSignature)
						{
							//New Function is created, create a new function symbol table
							SymbolTable funcSymbolTable = new SymbolTable (currentSymTable);
							currentSymTable.children.Add (funcSymbolTable);
							currentSymTable = funcSymbolTable;
						}
						else if (newNode is FieldDeclaration)
						{
							currentSymTable.dict.Add (((FieldDeclaration)newNode).name,
							                          ((FieldDeclaration)newNode).type);
						}
						else if (newNode is ClassSignatureWithParentNode)
						{
							SymbolTable classSymbolTable = new SymbolTable (currentSymTable);
							currentSymTable.children.Add (classSymbolTable);
							ClassType classType = new ClassType (((ClassSignatureNode)newNode).classID.id,
							                                     classSymbolTable, 
							                                     (ClassType)symTableTree.getType (((ClassSignatureWithParentNode)newNode).parentID.id));
							classSymbolTable.parentType = classType;
							currentSymTable.dict.Add (((ClassSignatureNode)newNode).classID.id,
							                          classType);
							currentSymTable = classSymbolTable;
						}
						else if (newNode is ClassSignatureNode)
						{
							//New Class is created, create a new class symbol table
							SymbolTable classSymbolTable = new SymbolTable (currentSymTable);
							currentSymTable.children.Add (classSymbolTable);
							ClassType classType = new ClassType (((ClassSignatureNode)newNode).classID.id,
							                                     classSymbolTable, null);
							classSymbolTable.parentType = classType;
							currentSymTable.dict.Add (((ClassSignatureNode)newNode).classID.id,
							                          classType);
							currentSymTable = classSymbolTable;
						}
						else if (newNode is ClassDefinitionNode)
						{
							//When definition of class is obtained load the parent symbol table
							currentSymTable = currentSymTable.parent;
						}

						//if (!SemanticAnalyzer.analyze (newNode, symTableTree, currentSymTable,
						//                               lexer.line, error))
						{
							//errors.Add (error);
						}
					}

					if (topNode != null)
					{
						nodeStack.Push (topNode);
					}

					stackStates.Push (g.gotoTable.getState (p.head.ToString (), stackStates.Peek ()));
					t = null;
				}

				else if (action.type == ActionTable.Action.ActionType.Accept)
				{
					Console.WriteLine ("peeeking \n");
					nodeStack.Peek ().travelNode ();
					//symTableTree.print ();
					return nodeStack.Peek ();
				}
			}
		}
	
//		void typeCheck (SymbolTable currSymTable, ASTNode node)
//		{
//			if (node is BinaryOperatorNode)
//			{
//				BinaryOperatorNode bnode = (BinaryOperatorNode)node;
//				if (bnode.op == Word.LogicalAnd || bnode.op == Word.LogicalOR)
//				{
//					ExprNode n1 = null, n2 = null;
//
//					foreach (ASTNode pnode in node.listNodes)
//					{
//						if (pnode is ExprNode)
//						{
//							if (n1 == null)
//								n1 = (ExprNode)(pnode);
//							else
//								n2 = (ExprNode)(pnode);
//						}
//					}
//
//					Type t1 = n1.getType (currSymTable);
//					Type t2 = n2.getType (currSymTable);
//	
//					if (t1 != Type.Integer)
//						n1.castTo = Type.Integer;
//
//					if (t2 != Type.Integer)
//						n2.castTo = Type.Integer;
//				}
//				else if (bnode.op == Word.Addition ||
//				         bnode.op == Word.Subtraction ||
//				         bnode.op == Word.Division ||
//				         bnode.op == Word.Multiply)
//				{
//					ExprNode n1 = bnode.expr1, n2 = bnode.expr2;
//
//					Type t1 = n1.getType (currSymTable);
//					Type t2 = n2.getType (currSymTable);
//					Type t = t1;
//
//					if (t1 != t2 && t1 != null && t2 != null)
//					{
//						t = Type.max (t1, t2);
//						if (t == null)
//						{
//							Console.WriteLine ("Error Types do not match");
//							return;
//						}
//
//						if (t != t1)
//							n1.castTo = t;
//						else
//							n2.castTo = t;
//					}
//				}
//				else if (bnode.op == Word.Equal)
//				{
//					ExprNode n1 = bnode.expr1, n2 = bnode.expr2;
//
//					Type t1 = n1.getType (currSymTable);
//					Type t2 = n2.getType (currSymTable);
//					Type t = t1;
//					if (t2 is ArrayType)
//					{
//						if (!(t1 is ArrayType))
//						{
//							Console.WriteLine ("Expected Array Type");
//							Environment.Exit (1);
//						}
//						else if (((ArrayType)t2).type != ((ArrayType)t1).type)
//						{
//							Console.WriteLine ("Expected array of " + ((ArrayType)t2).type.tag);
//							Environment.Exit (1);
//						}
//					}
//
//					if (t1 != t2)
//					{
//						t = Type.min (t1, t2);
//						n2.castTo = t;
//					}
//				}
//			}
//
//			else if (node is FunctionCall)
//			{
//				IDNode idnode = (IDNode)node.listNodes [0];
//				FunctionType type = (FunctionType)idnode.getType (currSymTable);
//				int i = 1, j = 0;
//				while (i < node.listNodes.Count && j < type.listParamTypes.Count)
//				{
//					if (node.listNodes [i] is FunctionCallArgument)
//					{
//						Type tnodetype = currSymTable.getType (node.listNodes [i].listNodes [0].ToString ());
//						if (type.listParamTypes [j] != tnodetype)
//						{
//							Type t = Type.max (type.listParamTypes [j], tnodetype);
//							if (t == type.listParamTypes [j])
//								((ExprNode)node.listNodes [i]).castTo = t;
//
//							else
//							{
//								Console.WriteLine ("Error with type expected " + type.listParamTypes [j]);
//								break;	
//							}
//						}
//						j++;
//					}
//					i++;
//				}
//			}
//
//			else if (node is IDArrayNode)
//			{
//				bool firstExprNode = false;
//				foreach (ASTNode n in node.listNodes)
//				{
//					if (n is ExprNode && firstExprNode == true)
//					{
//						if (((ExprNode)n).getType (currSymTable) != Type.Integer)
//							Console.WriteLine ("Warning: Array subscript is not an Integer");
//
//						break;
//					}
//					else if (n is ExprNode)
//					{
//						firstExprNode = true;
//					}
//				}
//			}
//		}
	}
}
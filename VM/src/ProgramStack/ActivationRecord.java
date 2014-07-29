package ProgramStack;

public class ActivationRecord 
{
	ActivationRecord callerRecord;
	ProgramCounter pc;
	LocalMemory local;
	LocalMemory global;

	public ActivationRecord (ProgramCounter pc, LocalMemory global,
							 ActivationRecord callerRecord)
	{
		this.pc = new ProgramCounter (pc.getValue() + 1);
		this.global = global;
		this.callerRecord = callerRecord;
		
	}
	public ProgramCounter getProgramCounter ()
	{
		return pc;
	}
}

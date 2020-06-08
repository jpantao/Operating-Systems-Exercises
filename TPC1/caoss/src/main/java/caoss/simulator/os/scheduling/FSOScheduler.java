package caoss.simulator.os.scheduling;

import java.util.Queue;
import java.util.concurrent.LinkedBlockingQueue;

import caoss.simulator.Program;
import caoss.simulator.configuration.Hardware;
import caoss.simulator.hardware.DeviceId;
import caoss.simulator.hardware.Timer;
import caoss.simulator.os.Dispatcher;
import caoss.simulator.os.Logger;
import caoss.simulator.os.ProcessControlBlock;
import caoss.simulator.os.Scheduler;



public class FSOScheduler implements Scheduler<SchedulingState> {

	private static final int QUANTUM = 10;
	private static final int QUOTA = 20;


	private final Queue<ProcessControlBlock<SchedulingState>>[] readyQueues;
	private final Queue<ProcessControlBlock<SchedulingState>> blockedQueue;
	private ProcessControlBlock<SchedulingState> running; // just one CPU
			
	private final Timer timer = (Timer) Hardware.devices.get(DeviceId.TIMER);

	
	@SuppressWarnings("unchecked")
	public FSOScheduler( ) {
		this.readyQueues = (Queue<ProcessControlBlock<SchedulingState>>[]) new LinkedBlockingQueue[2];
		this.blockedQueue = new LinkedBlockingQueue<ProcessControlBlock<SchedulingState>>();
		
		for (int i = 0; i < 2; i++) 
			this.readyQueues[i] = new LinkedBlockingQueue<ProcessControlBlock<SchedulingState>>();

	}
	
	

	@Override
	public synchronized void newProcess(Program program) {	
	
		ProcessControlBlock<SchedulingState> pcb =
				new ProcessControlBlock<SchedulingState>(program, 
						new SchedulingState(0, QUOTA));
		Logger.info("Create process " + pcb.pid + " to run program " + program.getFileName());
		
		if (this.running == null) 
			dispatch(pcb, QUANTUM);
		else 
			this.readyQueues[pcb.getSchedulingState().getLevel()].add(pcb);
		logQueues();
	}

	@Override
	public synchronized void ioRequest(ProcessControlBlock<SchedulingState> pcb) {
		Logger.info("Process " + pcb.pid + ": IO request");

		this.blockedQueue.add(pcb);
		chooseNext();		
		logQueues();
	}


	@Override
	public synchronized void ioConcluded(ProcessControlBlock<SchedulingState> pcb) {
		Logger.info("Process " + pcb.pid + ": IO concluded");

		SchedulingState state = pcb.getSchedulingState();
		state.setQuota(QUOTA);
		state.setLevel(0);
		this.blockedQueue.remove(pcb);
		if (this.running == null) 
			dispatch(pcb, QUANTUM);
		else{
			this.readyQueues[state.getLevel()].add(pcb);
		} 
			
		
		logQueues();
	}

	@Override
	public synchronized void quantumExpired(ProcessControlBlock<SchedulingState> pcb) {
		Logger.info("Process " + pcb.pid + ": quantum expired");
		
		SchedulingState state  =  pcb.getSchedulingState();
		if(state.getLevel() == 0){
			state.setQuota(state.getQuota() - QUANTUM);
			if(state.getQuota() <= 0){
				Logger.info("Process " + pcb.pid + ": quota expired");
				state.setQuota(QUOTA);
				state.setLevel(1);
			}
		}
		this.readyQueues[state.getLevel()].add(pcb);
		chooseNext();	
		logQueues();
	}

	@Override
	public synchronized void processConcluded(ProcessControlBlock<SchedulingState> pcb) {
		Logger.info("Process " + pcb.pid + ": execution concluded");
		
		long turnarround = Hardware.clock.getTime() - pcb.arrivalTime;
		Logger.info("Process " + pcb.pid + ": turnarround time: " + turnarround);		
		chooseNext();
		logQueues();
	}
		
	private void chooseNext() {
		if(!this.readyQueues[0].isEmpty()){
			dispatch(this.readyQueues[0].poll(), QUANTUM);
		}else if(!this.readyQueues[1].isEmpty()){
			dispatch(this.readyQueues[1].poll(), QUANTUM * 2);
		}else{
			dispatch(null, 0);
		}
	}
	
	private void dispatch(ProcessControlBlock<SchedulingState> pcb, int quantum) {
		Dispatcher.dispatch(pcb);
		running = pcb;
		
		if (pcb != null) {
			SchedulingState state = pcb.getSchedulingState();
			state.setSchedulingTime(Hardware.clock.getTime());
			timer.set(quantum);
			Logger.info("Run process "+ pcb.pid +" (quantum="+ quantum +", quota="+ state.getQuota()+")");
		}
		else
			timer.set(0);
	}
	
	
	private void logQueues() {
		int i = 0;
		for (Queue<ProcessControlBlock<SchedulingState>> queue : this.readyQueues) {
			Logger.info("Queue " + i + ": " + queue);
			i++;
		}
		Logger.info("Blocked " + blockedQueue);
	}
}

/*****************************************************************************
jnt.Bench.BenchApplet
 *****************************************************************************/
package jnt.Bench;
import java.awt.*;
import java.net.URL;
import java.util.Vector;

/**
 jnt.Bench.Applet provides an Applet or Application with GUI to display and execute
 a benchmark implementing jnt.Bench.Target.

@author Bruce R. Miller (bruce.miller@nist.gov)
@author Contribution of the National Institute of Standards and Technology,
@author not subject to copyright.
*/

public class Applet extends java.applet.Applet {
  Bench bench = null;
  int segment = 0;		// -1 for showing Table, otherwise, segment index.
  Choice chooser;
  Button executeButton, abortButton, submitButton;
  Plotter plotter;
  Panel centerPanel;
  CardLayout flipper;
  List table;
  Thread thread;
  Label status;
  
  Color getColor(String parmname, Color defalt){
    try { 
      return new Color(Integer.parseInt(getParameter(parmname),16));
    } catch(Exception e){}
    return defalt; }

  public void init() {
    setBackground(getColor("BGCOLOR",getBackground()));
    setForeground(getColor("TEXT",getForeground()));
    Color buttonbackground= getColor("BUTTON_BGCOLOR",getBackground());
    Color buttonforeground= getColor("BUTTON_TEXT",getForeground());

    Font font=getFont();
    if (font == null) font = new Font("Helvetica",Font.PLAIN,12);
    GridBagLayout bag = new GridBagLayout();
    GridBagConstraints cnst = new GridBagConstraints();
    setLayout(bag);

    Label title = new Label("" ,Label.CENTER);
    title.setFont(new Font(font.getName(),Font.BOLD,font.getSize()+2));
    cnst.fill = GridBagConstraints.HORIZONTAL;
    cnst.gridwidth=GridBagConstraints.REMAINDER;
    bag.setConstraints(title,cnst);
    add(title);
    chooser = new Choice();
    chooser.setForeground(buttonforeground);
    chooser.setBackground(buttonbackground);
    bag.setConstraints(chooser,cnst);
    add(chooser);
    centerPanel = new Panel();
    cnst.fill = GridBagConstraints.BOTH;
    cnst.weightx=cnst.weighty=1;
    bag.setConstraints(centerPanel,cnst);    
    add(centerPanel);
    centerPanel.setLayout(flipper = new CardLayout());
    centerPanel.add("graph",plotter = new Plotter());
    centerPanel.add("table",table = new List());
    Color plotterBG = getColor("PLOTTER_BGCOLOR",Color.white);
    Color plotterFG = getColor("PLOTTER_TEXT",Color.black);
    plotter.setPlotterColor(plotterBG);
    plotter.setLineColor(plotterFG);
    plotter.setBarColor(getColor("PLOTTER_BAR",Color.yellow));
    plotter.setSpecialColor(getColor("PLOTTER_SPECIAL",Color.red));
    table.setBackground(plotterBG);
    table.setForeground(plotterFG);
    table.setFont(new Font("Courier",Font.PLAIN,font.getSize()));

    executeButton=new Button("Execute Benchmark");
    executeButton.setForeground(buttonforeground);
    executeButton.setBackground(buttonbackground);
    cnst.weightx=1; cnst.weighty=0;
    cnst.fill = GridBagConstraints.HORIZONTAL;
    cnst.gridwidth=1;
    bag.setConstraints(executeButton,cnst);
    add(executeButton);

    abortButton  =new Button("ABORT");
    abortButton.setForeground(buttonforeground);
    abortButton.setBackground(buttonbackground);
    abortButton.disable();
    bag.setConstraints(abortButton,cnst);
    add(abortButton);

    submitButton  =new Button("Submit Results");
    submitButton.setForeground(buttonforeground);
    submitButton.setBackground(buttonbackground);
    submitButton.disable();
    cnst.gridwidth=GridBagConstraints.REMAINDER;
    bag.setConstraints(submitButton,cnst);
    add(submitButton);

    status = new Label("",Label.CENTER);
    status.setFont(new Font(font.getName(),Font.BOLD,font.getSize()));
    bag.setConstraints(status,cnst);
    add(status);

    if (bench == null) {	//  Get descriptor parameter (for Applet), then try read
      try {
	String desc = getParameter("descriptor");
	if (desc == null) desc="default.descriptor";
	bench = new Bench(this,new URL(getDocumentBase(),desc));
      } catch(Exception e) { 
	status.setText(e.toString());
	e.printStackTrace(System.out); }}

    if (bench != null) {	// Got Bench descriptor? final initialization
      title.setText(bench.getName());
      String names[] = bench.getSegmentNames();
      chooser.addItem(names[0]); // First segment first, always
      chooser.addItem("Show Table"); // Table prominent, always
      for(int i=1; i<names.length; i++)	// Remaining segments follow.
	chooser.addItem(names[i]);
      if(!bench.isRunnable()) executeButton.disable();
      doDisplay(); }
    else {			// else, setup for error report.
      executeButton.disable(); }
  }

  public void stop() {
    abortBenchmark(); }		// kill the benchmarking process, if any.

  /** Handling Events */
  public boolean handleEvent(Event e) {
    if((e.id == Event.ACTION_EVENT) && (e.target == executeButton))    
      startBenchmark();
    else if((e.id == Event.ACTION_EVENT) && (e.target == abortButton))
      abortBenchmark();
    else if((e.id == Event.ACTION_EVENT) && (e.target == submitButton)) 
      submitBenchmark();
    else if((e.id == Event.ACTION_EVENT) && (e.target == chooser)) {
      int c = chooser.getSelectedIndex(); // Translate Chosen item into segment number
      if (c == 1) c = -1;	// Show Table
      else if (c > 1) c--;
      if (c != segment) {
	segment=c;
	doDisplay(); 
	status.setText(""); }}
    else if (((e.id == Event.LIST_SELECT) ||(e.id == Event.LIST_DESELECT)) 
	     && (e.target == table)) {
      // Hack to keep the `special' row selected (ie. Highlighted!!!)
      int p = bench.getSpecialRownum(); 
      if (p >= 0) table.select(p); 
      status.setText(""); }
    else
      return false;
    return true; }

  void startBenchmark() {
    if (thread == null) {
      status.setText("Running Benchmark");
      status.setForeground(Color.red);
      waitCursor();
      abortButton.enable();
      thread = new Thread(bench);
      thread.start(); }}

  void abortBenchmark() {
    if (thread != null) {
      status.setForeground(getForeground());
      thread.stop(); 
      thread = null; }}

  /** Callback from Bench telling us the benchmark is done.*/
  void benchmarkDone(boolean stat, String result) {
    status.setText(result);
    status.setForeground(getForeground());
    revertCursor();
    abortButton.disable();
    submitButton.enable(stat && (bench.isSubmittable()));
    doDisplay();    
    thread = null; }

  void submitBenchmark() {
    new SubmitDialog(this,bench);  }

  /** Callback from SubmitDialog to report success or failure */
  void submissionDone(boolean stat, String result) {
    status.setText(result);
    submitButton.enable(!stat);
  }

  /** Callback to inform the applet of interesting status. */
  public void noteStatus(String stat){
    status.setText(stat); }

  /* **********************************************************************
    The next two methods support a `wait' cursor during execution */
  int cursor = -1;
  Frame parent = null;
  void waitCursor() {
    if (parent == null) {	// In 1.0.2 can only set cursor of Frames!
      Component p = getParent();
      while ((p != null) && !(p instanceof Frame))
	p = p.getParent();
      parent = (Frame) p; }
    if (parent != null) {
      cursor = parent.getCursorType();
      parent.setCursor(Frame.WAIT_CURSOR); }}

  void revertCursor() {
    if (cursor != -1)
      parent.setCursor(cursor); }

  /** **********************************************************************
    Display the graph or table of results */
  void doDisplay() {
    if (segment == -1) {		// Show table
      flipper.show(centerPanel,"table");
      if (table.countItems() > 0) 
	table.clear();
      String rows[] = bench.tableRows();
      for(int i=0; i<rows.length; i++) 
	table.addItem(rows[i]);
      int p = bench.getSpecialRownum(); 
      if (p >= 0) table.select(p); 
    } else {
      flipper.show(centerPanel,"graph");
      plotter.setData(bench.getEntries(), bench.getSegmentValues(segment),
		      bench.getSegmentUnits(segment),bench.getSpecialPos()); }
  }

  /** **********************************************************************
      Running as application
  */
  public static void main(String args[]) {
    new AppletFrame((args.length >= 1 ? args[0] : "default.descriptor")); }
}

class AppletFrame extends Frame {
  public AppletFrame(String descriptor) {
    super("BenchMarker"); 
    try {
      Applet app = new Applet();
      app.bench = new Bench(app,descriptor);
      add(app);
      resize(500,360);
      app.init();
      show();
      app.start();
    } catch(Exception e) {
      e.printStackTrace(System.out); }}

  public boolean handleEvent(Event e) {
    if (e.id == Event.WINDOW_DESTROY) {
      dispose();
      System.exit(0);
      return true; }
  return false;}
}

/*****************************************************************************
jnt.Bench.Bench
 *****************************************************************************/
package jnt.Bench;
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.StreamTokenizer;
import java.io.IOException;
import java.io.PrintStream;
import java.io.FileInputStream;
import java.util.Vector;
import java.net.URL;

/** 
A Description of a Benchmark.

Put description of Descriptor file here!!!

@author Bruce R. Miller (bruce.miller@nist.gov)
@author Contribution of the National Institute of Standards and Technology,
@author not subject to copyright.
*/
public class Bench implements Runnable {
  String name = "A BenchMark"; // Pretty name for the Benchmark
  String targetClass=null;	// Name of a Class implementing Benchmark
  String subemail=null;		// Email to submit results to.
  String suburl=null;		// URL to post results to.
  String units = "Seconds";	// Default units of each segment.
  int decimals = 0;		// Default # of decimals to print in values.

  Segment segments[] = new Segment[1];
  Vector entries = new Vector(); // Results for each Tested system
  double current[];		// results of current measurement.

  Applet applet = null;		// GUI that called us -- if any!
  boolean ascending = true;	// Is segment[0] sorted in ascending order?
  int specpos = -1;		// position of the special system in entries

  /** Create a Benchmark Bench based on the description in a file.*/
  public Bench(Applet applet, String filename) throws IOException{
    this(applet,new FileInputStream(filename)); }

  /** Create a Benchmark Bench based on the description in a URL.*/
  public Bench(Applet applet, URL url) throws IOException{
    this(applet,url.openStream()); }

  public Bench(Applet applet, InputStream stream) throws IOException{
    this.applet = applet;
    parseDescriptor(stream); }

  /* ______________________________________________________________________
    Parsing the Benchmark Descriptor */

  void parseDescriptor(InputStream stream) throws IOException {
    StreamTokenizer in =  new StreamTokenizer(stream);
    in.commentChar('#');
    in.quoteChar('"');
    in.wordChars('_','_');
    in.whitespaceChars(',',',');
    in.whitespaceChars(';',';');
    in.parseNumbers();
    while(in.nextToken()==StreamTokenizer.TT_WORD){
      String var = in.sval;
      getPunct(in,'=');
      if     (var.equals("name"))        name        = parseString(in,"name");
      else if(var.equals("target"))      targetClass = parseString(in,"target");
      else if(var.equals("submission_email")) subemail = parseString(in,"submission_email");
      else if(var.equals("submission_url")) suburl = parseString(in,"submission_url");
      else if(var.equals("units"))       units       = parseString(in,"units"); 
      else if(var.equals("decimals"))    decimals    =(int)parseNumber(in,"decimals");
      else if(var.equals("segments"))     parseSegments(in);
      else if(var.equals("entries"))     parseEntries(in);
      else throw new IOException("Unknown parameter "+var); }
    checkDefaultSegment();
  }

  void checkDefaultSegment(){
    if (segments[0]==null)	// Add a default segment, in case none get defined.
      segments[0]= new Segment(name,null,units,decimals); }

  void getPunct(StreamTokenizer in, char c) throws IOException {
    if(in.nextToken() != c) parseError(in,""+c,"char"); }

  void parseSegments(StreamTokenizer in) throws IOException {
    Vector segs = new Vector();
    getPunct(in,'(');
    int tok;
    while((tok=in.nextToken()) != ')'){
      String name="";
      String u=units;
      String s=null;
      int d=decimals;
      if ((tok==StreamTokenizer.TT_WORD)||(tok=='"'))
	name = in.sval;
      else if(tok=='('){
	name=parseString(in,"Segment Name");
	while((tok=in.nextToken())==StreamTokenizer.TT_WORD){
	  String var=in.sval;
	  getPunct(in,'=');
	  if     (var.equals("units"))     u = parseString(in,"units"); 
	  else if(var.equals("decimals"))  d =(int)parseNumber(in,"decimals");
	  else if(var.equals("shortname")) s = parseString(in,"shortname");
	}
	if(tok != ')') parseError(in,")","char"); }
      else parseError(in,"( or segment name","");
      segs.addElement(new Segment(name,s,u,d)); }
    segments = new Segment[segs.size()];
    segs.copyInto(segments);
  }

  void parseEntries(StreamTokenizer in) throws IOException {
    checkDefaultSegment();
    double v0=0.0;
    getPunct(in,'(');
    int row=0;
    while(in.nextToken()=='('){
      double v[] = new double[segments.length];
      entries.addElement(new Entry(parseString(in,"Entry Name"),v));
      for(int seg=0; seg<segments.length; seg++)
	v[seg] = parseNumber(in,"Segment Value");
      if (row > 0) ascending = ascending && (v0 < v[0]);
      v0 = v[0];
      getPunct(in,')'); 
      row++; }
    in.pushBack();
    getPunct(in,')');
  }

  void parseError(StreamTokenizer in, String what, String type) throws IOException {
    throw new IOException("Parsing error on line "+in.lineno()
			  +",\n Expecting "+what+" ("+type+")"+
			  " but got "+in.toString()); }

  double parseNumber(StreamTokenizer in, String what) throws IOException {
    if (in.nextToken() != StreamTokenizer.TT_NUMBER)
      parseError(in,what,"number");
    return in.nval; }

  String parseString(StreamTokenizer in, String what) throws IOException {
    if ((in.nextToken() != '"') && (in.ttype != StreamTokenizer.TT_WORD)) 
      parseError(in,what,"string");
    return in.sval; }

  /** **********************************************************************
      Accessors for the Segment & Entry information. */

  /** Return the name of the benchmark target.*/
  public String getName()        { return name; }
  /** Return whether the benchmark Target is (likely) runnable. */
  public boolean isRunnable() { return targetClass != null; }
  /** Return whether the benchmark results are submittable. */
  public boolean isSubmittable() { return (subemail != null)||(suburl != null); }
  /** Return the email address that submissions should be sent to.*/
  public String getSubmissionsEmail() { return subemail; }
  /** Return the url that submissions should be posted to.*/
  public String getSubmissionsURL() { return suburl; }
  /** Return the index of the Special Entry (the current system). */
  public int getSpecialPos()     { return specpos; }
  /** Return the number of segments in this benchmark.*/
  public int getNumSegments()     { return segments.length; }

  /** Return the names of the segments in this benchmark. */
  public String[] getSegmentNames() { 
    String names[]=new String[segments.length];
    for(int i=0; i < names.length;i++)
      names[i]=segments[i].name;
    return names; }
    
  /** Return the units of segment seg in this benchmark. */
  public String getSegmentUnits(int seg) {
    return segments[seg].units; }

  /** Return the units of the segments in this benchmark. */
  public String[] getSegmentUnits() {
    String units[]=new String[segments.length];
    for(int i=0; i < units.length;i++)
      units[i]=segments[i].units;
    return units; }

  /** Return the measured values for all entries in segment seg in this benchmark. */
  public double[] getSegmentValues(int seg) {
    double vals[]=new double[entries.size()];
    for(int row=0; row < vals.length; row++)
      vals[row] = ((Entry)entries.elementAt(row)).values[seg];
    return vals; }

  /** Return the platform names for all entries in this benchmark. */
  public String[] getEntries() {
    String entr[]=new String[entries.size()];
    for(int i=0; i< entr.length;i++)
      entr[i]=((Entry)entries.elementAt(i)).platform;
    return entr; }

  /** Return the measured values for the current measurement (if any) 
    * of this benchmark. */
  public double[] getCurrentValues() { return current; }

  /* ______________________________________________________________________
    Execution of the Benchmark Target */

  Stopwatch timer[];
  boolean timerAPIused=false;
  void doExecution() throws Exception {
    Target target = (Target) Class.forName(targetClass).newInstance();
    Stopwatch overall= new Stopwatch();
    timer = new Stopwatch[segments.length];
    for(int i=0; i < segments.length; i++)
      timer[i]=new Stopwatch();
    System.gc();
    overall.start();
    current =target.execute(this);
    overall.stop();
    if (current == null){	// Target didn't return timings...we'll create them
      current=new double[segments.length];
      if(timerAPIused){		// the timer API was used, extract those timings.
	for(int i=0; i<segments.length; i++) 
	  current[i]=timer[i].read(); }
      else {			// Otherwise, just use the overall timing.
	current[0]=overall.read(); }}
    insertEntry(current); }

  void prepTimer(int segment){
    timerAPIused=true;
    noteStatus("GC'ing");
    System.gc();
    noteStatus("Executing: "+segments[segment].name);
    Thread.currentThread().yield();
  }

  /** Start the timer for segment.  It resets the timer for this segment
    * if any previous measurement had been made. 
    * This is intended to be used as a callback from Target. */
  public void startTimer(int segment) {
    prepTimer(segment);
    timer[segment].start(); }

  /** Stop the timer for this segment.
    * It returns the timing, up to this point.
    * This is intended to be used as a callback from Target. */
  public double stopTimer(int segment){
    double time = timer[segment].stop();
    noteStatus("done");
    return time; }

  /** Reset the timer for this segment.
    * This is intended to be used as a callback from Target. */
  public void resetTimer(int segment){
    timer[segment].reset(); }

  /** Resume the timer for segment.  This is like starting the timer without resetting
    * it first.
    * This is intended to be used as a callback from Target. */
  public void resumeTimer(int segment){
    prepTimer(segment);
    timer[segment].resume(); }

  /** Callback to inform us of interesting status. */
  public void noteStatus(String stat){
    if (applet != null)
      applet.noteStatus(stat); }

  void insertEntry(double values[]){
    if (specpos >= 0)	// Remove previous special data, if any
      entries.removeElementAt(specpos);
    int seg=0;			// Segment to sort on.
    int n=entries.size();
    double v0=values[seg];
    if (ascending) {
      for(specpos=0; specpos<n; specpos++) // Find approp. position
	if(v0 <((Entry)entries.elementAt(specpos)).values[seg]) break;}
    else {
      for(specpos=0; specpos<n; specpos++) // Find approp. position
	if(v0 >((Entry)entries.elementAt(specpos)).values[seg]) break;}
    entries.insertElementAt(new Entry(">>Your Computer<<",values),specpos);
  }

  public void run()  {
    try {			// Try to lower the priority 1 click.
      Thread.currentThread().setPriority(Thread.currentThread().getPriority()-1);
    } catch(Exception e){};
    boolean status = false;
    String message="";
    ThreadDeath rethrow = null;
    try { 
      doExecution();
      status = true;
      message = "Benchmark Done"; }
    catch (ThreadDeath ex) {
      message = "Benchmark Aborted"; 
      rethrow = ex; }
    catch (Throwable ex) {
      message = ex.toString();
      ex.printStackTrace(System.out); }
    if (applet !=null) 
      applet.benchmarkDone(status,message); // Callback says we're done
    if (rethrow != null)	// Need to  propogate this!
      throw(rethrow); 
  }

  /* ______________________________________________________________________
    Printing Results of measurements */

  public void printMeasurements(PrintStream out){
    String table[]= getSegmentNames();
    int m=0;
    for(int i=0; i<segments.length; i++) 
      if (segments[i].decimals>m) m=segments[i].decimals;
    Formatter.addColumn(table,Formatter.format(current,m),1);
    Formatter.addColumn(table,getSegmentUnits(),0);
    for(int i=0; i<table.length; i++)
      out.println(table[i]); 
  }

  public int getSpecialRownum() { 
    return (specpos >= 0 ? 2 + specpos : -1); }

  /** Return an array of strings representing the rows of a table tabulating 
    * the values in the benchmark. */
  public String[] tableRows() {
    String headings[] = new String[2];
    headings[0]=headings[1]="";
    String table[] = Formatter.conc(headings,getEntries());
    for(int i=0; i<segments.length; i++){
      headings[0]=segments[i].shortname;
      headings[1]=segments[i].units;
      Formatter.addColumn(table,Formatter.conc(headings,
					       Formatter.format(getSegmentValues(i),
								segments[i].decimals)),1);}
    return table; }

  /* ______________________________________________________________________*/

  /** Run a Benchmark as a command-line application (without a GUI)  */
  public static void main(String args[]) throws Exception {
    Bench bench = new Bench(null,(args.length >= 1 ? args[0] : "default.descriptor"));
    bench.doExecution();	// Run the Target in Current thread!
    bench.printMeasurements(System.out); }
}

class Segment {
  String name;
  String shortname;
  String units;
  int decimals;
    
  /** Create a Segment description describing a particular measurement segment.*/
  public Segment(String name, String shortname, String units, int decimals){
    this.name=name;
    this.shortname= (shortname !=null ? shortname : name);
    this.units=units;
    this.decimals=decimals;  }
}

class Entry{
  String platform;
  double values[];

  /** Create an Benchmark Entry describing measurements made on a particular platform.*/
  public Entry(String platform, double values[]){
    this.platform=platform;
    this.values=values; }
}

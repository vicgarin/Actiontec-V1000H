/*****************************************************************************
jnt.Bench.SubmitDialog
 *****************************************************************************/
package jnt.Bench;
import java.awt.*;
/**
   SubmitDialog provides a Dialog box for filling in a benchmark submission.
   It extracts the relevent information from System.Properties of the VM, and
   provides text boxes for filling in other information.
   It uses Bench.SendMail to send the message, and uses callbacks to the Applet (if any)
   to inform of the success or failure of the submission.
*/
public class SubmitDialog extends Frame {
  TextComponent user,from, brand,cpu, os, vm,comments;
  Label status;
  Button submit,cancel;
  Applet applet;
  Bench bench;

  GridBagLayout layout = new GridBagLayout();
  GridBagConstraints constraints = new GridBagConstraints(); 

  /** Create a SubmitDialog to report on a new measurement of the Benchmark Target
    * described in bench.  */
  public SubmitDialog(Applet applet, Bench bench) {
    super("Benchmark Submission");
    this.bench = bench;
    this.applet = applet;
    setForeground(applet.getForeground());
    setBackground(applet.getBackground());
    Font font=getFont();
    if (font == null) font = new Font("Helvetica",Font.PLAIN,12);

    setLayout(layout);

    Label label;
    label = new Label("Submit Benchmark Results" ,Label.CENTER);
    label.setFont(new Font(font.getName(),Font.BOLD,font.getSize()+2));
    constraints.weightx=1; constraints.weighty=0;
    constraints.fill = GridBagConstraints.HORIZONTAL;
    constraints.gridwidth=GridBagConstraints.REMAINDER;
    layout.setConstraints(label,constraints);
    add(label);

    user = addField("Your Name:",1,"");
    from = addField("Your Email:",1,"");
    brand= addField("Computer Brand:",1,"");
    cpu  = addField("CPU type & Speed:",1,"");
    os   = addField("OS name & version:",1,
		    getSysProp("os.name")+" "+getSysProp("os.version"));
    vm   = addField("Java VM name & version:",1,
		    getSysProp("java.vendor")+" "+getSysProp("java.version"));
    comments=addField("Comments:",3,"");

    submit    = new Button("Send");
    constraints.gridwidth=1;
    layout.setConstraints(submit,constraints);
    add(submit);
    cancel    = new Button("Cancel");
    constraints.gridwidth=GridBagConstraints.REMAINDER;
    layout.setConstraints(cancel,constraints);
    add(cancel);
  
    status = new Label("");
    constraints.gridwidth=GridBagConstraints.REMAINDER;
    layout.setConstraints(status,constraints);
    add(status);
  
    pack();
    show();
  }

  TextComponent addField(String name, int lines, String init) {
    Label label = new Label(name);
    constraints.gridwidth=1;
    layout.setConstraints(label,constraints);
    add(label);

    TextComponent field;
    if (lines==1) field= new TextField(40);
    else field = new TextArea(lines,40);
    constraints.gridwidth=GridBagConstraints.REMAINDER;
    layout.setConstraints(field,constraints);
    field.setText(init);
    add(field);
    return field; }
  
  String getSysProp(String property) {
    try {
      return System.getProperty(property);
    } catch(Exception e){}
    return "N/A";}

  /* attribute values are delimited by a `:' at the beginning, and a ';' at the end.
     Thus, semicolons in the values are be changed to , */
  String clean(String string) {
    return string.replace(';',',');  }

  String constructBody () {
    StringBuffer buf = new StringBuffer();

    buf.append("START : **************************************** ;\n");
    buf.append("Benchmark.name    : "+clean(bench.getName()) +";\n");
    double vals[] = bench.getCurrentValues();
    buf.append( "Benchmark.Values  : ");
    if (vals != null) {
      for(int i=0; i<vals.length; i++) {
	buf.append(" "+ vals[i]); }}
    buf.append(";\n");
    buf.append( "  sys.os.name     : "+clean(getSysProp("os.name"))+";\n");
    buf.append( "  sys.os.version  : "+clean(getSysProp("os.version"))+";\n");
    buf.append( "  sys.os.arch     : "+clean(getSysProp("os.arch"))+";\n");
    buf.append( "  sys.java.vendor : "+clean(getSysProp("java.vendor"))+";\n");
    buf.append( "  sys.java.version: "+clean(getSysProp("java.version"))+";\n");
    buf.append( "  user.name       : "+clean(user.getText()) + ";\n");
    buf.append( "  user.email      : "+clean(from.getText()) + ";\n");
    buf.append( "  user.computer   : "+clean(brand.getText()) + ";\n");
    buf.append( "  user.CPU        : "+clean(cpu.getText()) + ";\n");
    buf.append( "  user.OS         : "+clean(os.getText()) + ";\n");
    buf.append( "  user.VM         : "+clean(vm.getText()) + ";\n");
    buf.append( "  user.Comments   : "+clean(comments.getText()) + ";\n");
    buf.append("END : **************************************** ;\n");
    return buf.toString();
  }
    
  boolean doCheck() {
    status.setText("Checking");
    if (from.getText().trim().length() == 0) { // Anything else to check?
      status.setText("Your Email address is required");
      from.requestFocus();
      return false; }
    return true; }

  void doSend() {
    if (!doCheck()) return;
    status.setText("Sending");
    String server = "";
    try {
	String recipient=bench.getSubmissionsEmail();
	String url=bench.getSubmissionsURL();
	if(recipient != null){
	  try { server = applet.getCodeBase().getHost(); } catch(Exception e) {}
	  if (server.length() == 0) {
	    server = recipient.substring(recipient.indexOf('@')+1); }
	  String subject = "BenchMark " + bench.getName() +" results";
	  SendMail.send(server,from.getText(),recipient,subject,constructBody()); }
	else if (url != null){
	  // IFF we are in an applet, try to use its server for submission
	  try { server = applet.getCodeBase().getHost(); 
	  } catch(Exception e) {}
	  if(server.length()!=0){ // Got a server? replace it into url.
	    int p=url.indexOf('/',7); // find / after host.
	    if(p>0)
	      url = "HTTP://" + server + url.substring(p); }
	  // So, now try to submit.
	  HTTPPost.post(url,constructBody()); }
	applet.submissionDone(true,"Submitted; Thank You!");
    } catch(Exception e) {
      e.printStackTrace(System.out); 
      applet.submissionDone(false,e.toString()); }
    dispose();
    }

  void doCancel(){
    applet.submissionDone(false,"Submission Cancelled");
    dispose(); }

  public boolean handleEvent(Event e) {
    if ((e.id == Event.WINDOW_DESTROY) ||
	((e.id == Event.ACTION_EVENT) && (e.target == cancel)))
      doCancel();
    else  if ((e.id == Event.ACTION_EVENT) && (e.target == submit))
      doSend();
    else if (e.id == Event.KEY_PRESS) {
      status.setText("");	// clear any messages if user types.
      return false; }		// but let this still be handled!
    else 
      return false;
    return true; }
}

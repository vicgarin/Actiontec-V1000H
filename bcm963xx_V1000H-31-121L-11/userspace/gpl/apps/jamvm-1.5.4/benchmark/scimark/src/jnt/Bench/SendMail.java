/*****************************************************************************
jnt.Bench.SendMail
 *****************************************************************************/
package jnt.Bench;
import java.net.Socket;
import java.net.ProtocolException;
import java.io.PrintStream;
import java.io.DataInputStream;
/**
SendMail implements a simple SMTP client to send mail. 

NOTE: Since a socket connection is used, when executed from an Applet,
the security model requires that the server be the same as the server 
from which the applet was loaded.

@author Bruce R. Miller (bruce.miller@nist.gov)
@author Contribution of the National Institute of Standards and Technology,
@author not subject to copyright.
*/
public class SendMail {

  final static int SMTPPort = 25;
  final static String CRLF="\r\n";

  /** Send an email message.
    * @param server The SMTP server
    * @param sender the sender's email address.
    * @param recipient the recipient's email address.
    * @param subject the subject of the message.
    * @param the message text. 
    */
  public static void send(String server, String sender, String recipient,
			  String subject, String message) throws Exception {
    if (server.trim().length() == 0)
      throw new ProtocolException("No SMTP Host given!");
    if (sender.trim().length() == 0)
      throw new ProtocolException("No SMTP sender given!");
    if (recipient.trim().length() == 0)
      throw new ProtocolException("No SMTP recipient given!");
    int i;			// Double any periods alone on a line!
    message += "\n";
    while((i=message.indexOf("\n.\n")) != -1)
      message = message.substring(0,i+2) + "." + message.substring(i+2);

    Socket socket   = new Socket(server, SMTPPort, true);
    PrintStream output   = new PrintStream(socket.getOutputStream());
    DataInputStream response = new DataInputStream(socket.getInputStream());
    check(response,220);
    docmd(output,"HELO " + server);
    check(response,250);
    docmd(output,"MAIL FROM: " + sender);
    check(response,250);
    docmd(output,"RCPT TO: " + recipient);
    check(response,251);
    docmd(output,"DATA");
    check(response,354);
    docmd(output,"Subject: "  + subject + CRLF+ CRLF + message + CRLF + ".");
    check(response,250);
    docmd(output,"QUIT");
    check(response,221);

    output.close();
    socket.close();
  }

  static void docmd(PrintStream output, String data) {
    data = data.trim();
    output.print(data + CRLF); }

  static void check(DataInputStream response, int code) throws Exception {
    String resp = response.readLine();
    int retcode = Integer.parseInt(resp.substring(0,3));

    if ((retcode == 250) || (retcode == code)) return;
    throw new ProtocolException(resp); }
}

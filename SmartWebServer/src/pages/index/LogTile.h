// -----------------------------------------------------------------------------------
// OnStep log tile
#pragma once

#include "../htmlHeaders.h"
#include "../htmlMessages.h"
#include "../htmlScripts.h"

extern void logTile(String &data);
extern void logTileGet();

const char html_logBox[] PROGMEM =
"<textarea id='log_text' readonly wrap='off' spellcheck='false' "
"style='width: 100%; height: 26em; text-align: left; font: 12px/1.4 Consolas, monospace; font-weight: normal; margin: 6px 0;'></textarea>"
"<div style='text-align: left;'>"
"<button type='button' onclick='logCopy()'>Copy</button>"
"<button type='button' onclick='logSave()'>Download</button>"
"<button type='button' onclick='logClear()'>Clear</button>"
"&nbsp;&nbsp;<label><input type='checkbox' id='log_auto' checked style='width: auto;'> Follow new lines</label>"
"</div>\n";

// polls log.txt every 2 seconds, the reply is "start,next,supported" then a newline and the log text
const char html_logScript[] PROGMEM =
"<script>\n"
"var logPos=0,logBusy=false;\n"
"function logPoll(){\n"
  "if(logBusy)return;logBusy=true;\n"
  "var r=new XMLHttpRequest();\n"
  "r.onreadystatechange=function(){\n"
    "if(r.readyState!=4)return;logBusy=false;if(r.status!=200)return;\n"
    "var t=r.responseText,nl=t.indexOf('\\n');if(nl<0)return;\n"
    "var h=t.slice(0,nl).split(','),start=+h[0],next=+h[1],txt=t.slice(nl+1);\n"
    "var a=document.getElementById('log_text');\n"
    "if(start<logPos)a.value='';\n"
    "if(start>logPos&&logPos>0)a.value+='\\n--- older text was dropped by the SmartWebServer ---\\n';\n"
    "if(txt.length){a.value+=txt;if(a.value.length>200000)a.value=a.value.slice(-150000);"
      "if(document.getElementById('log_auto').checked)a.scrollTop=a.scrollHeight;}\n"
    "logPos=next;\n"
    "document.getElementById('log_sta').innerHTML=(h[2]=='1')?(a.value.split('\\n').length-1)+' lines':'not supported by OnStep';\n"
    "if(txt.length>=4000)setTimeout(logPoll,50);\n"
  "};\n"
  "r.open('GET','log.txt?from='+logPos+'&nocache='+Math.random(),true);r.send(null);\n"
"}\n"
"function logCopy(){var a=document.getElementById('log_text');a.focus();a.select();try{document.execCommand('copy');}catch(e){}}\n"
"function logSave(){var a=document.getElementById('log_text'),b=new Blob([a.value],{type:'text/plain'}),l=document.createElement('a');"
  "l.href=URL.createObjectURL(b);l.download='onstep-log-'+new Date().toISOString().slice(0,19).replace(/[:T]/g,'-')+'.txt';"
  "document.body.appendChild(l);l.click();l.remove();}\n"
"function logClear(){if(!confirm('Clear the log?'))return;var r=new XMLHttpRequest();"
  "r.open('GET','log.txt?clear=1&nocache='+Math.random(),true);r.send(null);document.getElementById('log_text').value='';}\n"
"setInterval(logPoll,2000);logPoll();\n"
"</script>\n";

// -----------------------------------------------------------------------------------
// OnStep log tile
#include "LogTile.h"

#include "../Pages.common.h"
#include "../../libApp/onStepLog/OnStepLog.h"

// create the related webpage tile
void logTile(String &data)
{
  char temp[240] = "";

  snprintf_P(temp, sizeof(temp), html_tile_text_beg, "calc(100% - 16px)", "10em", "OnStep Log");
  data.concat(temp);
  data.concat(F("<div style='float: right; text-align: right;' id='log_sta' class='c'>...</div><br /><hr>"));
  data.concat(FPSTR(html_logBox));
  data.concat(FPSTR(html_logScript));
  data.concat(FPSTR(html_tile_end));
  www.sendContentAndClear(data);
}

// log text from position "from" on, or clear the log
void logTileGet()
{
  if (www.arg("clear").equals("1")) onStepLog.clear();

  uint32_t from = strtoul(www.arg("from").c_str(), NULL, 10);
  uint32_t start;
  String text = "";
  uint32_t next = onStepLog.read(from, text, 4096, &start);

  String data = String(start) + "," + String(next) + "," + (onStepLog.supported ? "1" : "0") + "\n";
  data.concat(text);

  www.sendHeader("Cache-Control", "no-cache");
  www.send(200, "text/plain", data);
}

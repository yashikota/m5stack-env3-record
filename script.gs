function doPost(e)
{
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getSheetByName('部屋1');

  // 送信されてくるJSONデータから、各要素を取り出す
  var params = JSON.parse(e.postData.getDataAsString());
  var hour = params.hour;
  var temp = params.temp;
  var humi = params.humi;
  var pressure = params.pressure;

  // データをシートに追加
  sheet.insertRows(2,1);                         // 2行1列目に列を挿入する
  sheet.getRange(2, 1).setValue(Utilities.formatDate(new Date(), 'JST', 'yyyy/MM/dd ' + hour + "時"));     // 受信日時を記録
  sheet.getRange(2, 2).setValue(temp);           // temp記録
  sheet.getRange(2, 3).setValue(humi);           // humi記録
  sheet.getRange(2, 4).setValue(pressure);       // pressure記録
}

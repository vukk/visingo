
/* ES6 module */

export function nthRoot(x, n) {
  try {
    var negate = n % 2 == 1 && x < 0;
    if(negate)
      x = -x;
    var possible = Math.pow(x, 1 / n);
    n = Math.pow(possible, n);
    if(Math.abs(x - n) < 1 && (x > 0 == n > 0))
      return negate ? -possible : possible;
  } catch(e){}
};

export function formatTime(time) {
  var min = parseInt(time / 6000);
  var sec = parseInt(time / 100) - (min * 60);
  var hundredths = me.pad(time - (sec * 100) - (min * 6000), 2);
  // trick: function (float) { return (float | 0) } -> <floored int>
  return (min > 0 ? me.pad(min, 2) : "00") + ":" + me.pad(sec, 2) + ":" + me.pad((hundredths | 0), 2);
};


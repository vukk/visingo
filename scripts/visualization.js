/*
 * Visingo visualization module
 *
 * Requires vis.js, jQuery and underscore/lodash, which are passed in to the
 * closure. Also requires jquery.timer.js, which should be loaded before this
 * script loaded.
 *
 * Note on the canvas resizing script:
 * Sometimes the canvas is the correct size, but is still the scrollbars
 * appear. Use CSS to forcibly hide the overflow and thus the scrollbars:
 * html,body {
 *   overflow: hidden;
 * }
 * In some cases "display: block;" is also required.
 *
 * Example on replacing the free height calculation function, using jQuery:
 * aspgv.calculateFreeHeight = function() {
 *     return window.innerHeight
 *         - $('#nav-main').height()
 *         - $('#nav-second').height()
 *         - 4; // NOTE: 4 pixel hack fix.
 * };
 * If the used height is static, ie. your UI elements have static height, then
 * you can just use aspgv.usedHeight = <used height>;
 * Example: aspgv.usedHeight = $('#nav-main-static').height();
 *
 * TODO:
 * - timing and solution JSONs could be combined
 *   - require only times and solutions, eg. optimums should be optional
 * - jslint
 * - use JSONP to avoid same origin policy problems
 * - could add a callback "init finished", where the user has to call startVis
 */

export nthRoot function (x, n) {
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

export formatTime function (time) {
    var min = parseInt(time / 6000);
    var sec = parseInt(time / 100) - (min * 60);
    var hundredths = me.pad(time - (sec * 100) - (min * 6000), 2);
    // trick: function (float) { return (float | 0) } -> <floored int>
    return (min > 0 ? me.pad(min, 2) : "00") + ":" + me.pad(sec, 2) + ":" + me.pad((hundredths | 0), 2);
};


// Visualization class for extension
// e.g. 'class ASPGraphVisualizer extends Visualization'

export default class Visualization {
  constructor(canvas, debugCanvas=false) {
    if (new.target === Visualization) {
      throw new TypeError("Cannot construct Visualization instances directly, extend it");
    }
    if (typeof this.start !== "function") {
      throw new TypeError("Visualization must implement method 'start'");
    }

    this.canvas = canvas;

    // default width and height
    //this.height = window.innerHeight;
    //this.width  = window.innerWidth;
    this.width  = this.canvas.width;
    this.height = this.canvas.height;

    //
    this.debugCanvas = debugCanvas;
  }

  startCanvasAutoResizing() {
      // Register an event listener to call the resizeCanvas() function each
      // time the window is resized.
      window.addEventListener('resize', this.resizeCanvas, false);
      // Resize for the first time
      this.resizeCanvasWrap();
  };

  resizeCanvasWrap() {
    resizeCanvas();
    if (this.debugCanvas) {
        this.drawCanvasRectangle();
    }
  };

  resizeCanvas() {
      // Recalculate
      this.width  = this.canvas.parent().width();
      this.height = this.canvas.parent().height();
      this.canvas.width  = this.width;
      this.canvas.height = this.height;
  };

  drawCanvasRectangle() {
    context = this.canvas.getContext('2d');
    context.strokeStyle = 'red';
    context.lineWidth = '1';
    context.strokeRect(0, 0, this.width, this.height);
  };

  // extending class must implement the following:
  /*
  // data given is the full solving data to play back, clasp/clingo JSON
  setPlaybackData(data) {};
  // data given is TODO
  setInitialData(data) {};
  // starts the visualization (playback)
  start() {};
  // called to add next answer set when doing interactive visualization
  // TODO: nononono, use a Visingo javascript module to subscribe to events
  //       or term (atom/predicate) streams.
  addAnswerSet(model) {};

  // TODO: need to separate playback vis and interactive vis, while
  //       supporting using modular codebase

  */


}

export class PlaybackVisualization extends Visualization {
  constructor(canvas) {
    if (new.target === PlaybackVisualization) {
      throw new TypeError("Cannot construct Visualization instances directly, extend it");
    }
    super(canvas);
  }
}

export class InteractiveVisualization extends Visualization {
  constructor(canvas) {
    if (new.target === InteractiveVisualization) {
      throw new TypeError("Cannot construct Visualization instances directly, extend it");
    }
    super(canvas);
  }
}

/* EOF */

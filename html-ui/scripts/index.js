
(function(document) {
    'use strict';

    var app = document.querySelector('#app');

    /* Defaults for bindings */
    app.visualizationType = 'playback';
    app.solverLogFile = [];
    app.visualizers = [];
    app.inputFiles = []; // NOTE: array
    app.parseStatus = 'outdated';

    // Listen for template bound event to know when bindings
    // have resolved and content has been stamped to the page
    app.addEventListener('dom-change', function() {
    });

    // See https://github.com/Polymer/polymer/issues/1381
    window.addEventListener('WebComponentsReady', function() {
        // imports are loaded and elements have been registered
    });

})(document);

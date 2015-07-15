/*
 * peg.js ASP term parser,
 * NOTE: very simplistic, strings in predicates might bite you
 */

/* initializer */

{
  var data = {};

  function makeInteger(o) {
    return parseInt(o.join(""), 10);
  }

  function extractList(list, index) {
    var result = [], i;

    for (i = 0; i < list.length; i++) {
      if (list[i][index] !== null) {
        result.push(list[i][index]);
      }
    }

    return result;
  }

  function buildList(first, rest, index) {
    return (first !== null ? [first] : []).concat(extractList(rest, index));
  }
}

/* start, return json */
start
  = predicate { return data; }


/* a single predicate */

predicate
  = predicateName:ident "(" inside:arguments ")"
    {
      data[predicateName] = {};
      data[predicateName][inside.length] = []
      data[predicateName][inside.length].push(inside);
    }

arguments
  = first:argument rest:("," S* argument)*
  {
    return buildList(first, rest, 2);
  }

argument
  = predicate
  / tag:ident
  / num:integer


/* numbers */

integer "integer"
  = digits:[0-9]+ { return makeInteger(digits); }


/* characters & strings */

// prefix allows default negation e.g. '-predicate(X,Y)'
ident "identifier"
  = prefix:$"-"? start:identStart chars:identChar* {
      return prefix + start + chars.join("");
    }

identStart
  = [a-z]i
  / nonascii

identChar
  = [_a-z0-9-]i
  / nonascii

nonascii
  = [\x80-\uFFFF]

S "whitespace"
  = [ \t\r\n\f]+

/*
 * peg.js ASP result term parser,
 * NOTE: very simplistic, no guarantee that all cases are covered...
 */

/* initializer */

{
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
  = terms:terms "\r"? "\n"? { return terms; }

terms
  = E* first:term rest:(E+ term)* E*
  { return buildList(first, rest, 1); }

term
  = predicate
  / atom:ident
  / aspstring
  / integer

/* a single predicate */

predicate
  = predicateName:ident "(" inside:arguments ")"
    {
      return {
        Type: "predicate",
        Name: predicateName,
        Arity: inside.length,
        Contains: inside
      };
    }

arguments
  = first:argument rest:("," E* argument)*
  {
    return buildList(first, rest, 2);
  }

argument
  = predicate
  / tag:ident
  / tuple:anontuple
  / string:aspstring
  / num:integer


/* tuples */

anontuple
  = "(" E* first:argument rest:("," E* argument)* E* ")"
  {
    return buildList(first, rest, 2);
  }

/* ASP strings */

aspstring "double quoted string"
  = "\"" str:string "\""
  {
    return {
      Type: "string",
      Contains: str
    };
  }

/* numbers */

posinteger "positive integer"
  = digits:[0-9]+ { return makeInteger(digits); }

integer "integer"
  = sign:"-"? digits:([0-9]+) { d = makeInteger(digits); return (sign === '-') ? -d : d }


/* characters & strings */

// not " or newline related characters
string "string"
  = str:([^\"\r\n\f]+) { return str.join("") }

// prefix allows default negation e.g. '-predicate(X,Y)'
ident "identifier"
  = prefix:$"-"? start:identStart chars:identChar* {
      return prefix + start + chars.join("");
    }

// allow "_" on the start of idents, since heuristic predicates might be present in the output
identStart
  = [_a-z]i
  / nonascii

identChar
  = [_a-z0-9-]i
  / nonascii

nonascii
  = [\x80-\uFFFF]

E "tab or space"
  = [ \t]+

S "whitespace"
  = [ \t\r\n\f]+

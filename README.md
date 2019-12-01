# Lila

A simple language that makes generating configuration files less painful.

## Motivation

There are many different domain specific languages used for generating configuration files. Jsonnet and helm to name a few. These languages suffer from two problems:

1. Strange, clunky syntax, making them hard to read and understand at a glance.
2. Lack of static typing, preventing easy discovery of the various configuration parameters.

Lila solves these problems by having a clean, familiar syntax that's easy to read and write. A strong typing system allows for static configuration type definition, helping library authors provide self-documenting configuration parameters. Additionally, Lila experiments with some new ideas around control flow that reduces cognitive overhead when branching while maintaing the intutive power of imperitive programming.

## Example

All Lila programs start with a `main` entry point:

```
// main.l

f main(input: None): None {

}
```

1. The `f` keyword denotes a function.
2. `f main(input: None): None` creates a function called `main` that takes in a variable called `input` of type `None` and produces `None` output. `None` is similar to the `void` type in other languages.
3. The program can be run with `lila main.l`

A more complete example:

```
// main.l

type Params {
    serviceName: String
    stages:      List<String>
}

type Service {
    serviceName: String
    stage:       String
}

f main(input: Params): List<Service> {
    return input
        .stages
        .filter((stage) -> { return stage != "test" })
        .map((stage) -> {
        return Service(input.serviceName, stage)
    })
}

// input.yaml
serviceName: My Service
stages:
  - dev
  - staging
  - test
  - prod
```

This program can be run with:

```
> lila main.l input.yaml
- serviceName: My Service
  stage: dev
- serviceName: My Service
  stage: staging
- serviceName: My Service
  stage: prod
```

Lila will automatically deserialize input.yaml into the main input parameter type, in this case `Params`. The output type is automatically converted to YAML and output to stdout. Lila supports both YAML and JSON input / output. The input format is determined by the file extension, but can be forced using the `--input-format [json|yaml]` flag. The output format matches the input format by default, but can be forced using the `--output-format [json|yaml]` flag.

## Language Guide

### Types

Types are a collection of variable state. There are specific with the syntax:

```
type <TypeName> {
    <FieldName>: <FieldTypeName>
}
```

Types can be instantiated with `<TypeName>()`

#### Built-in Types

Lila provides a small set of built-in types:

1. String
2. Int
3. Bool
4. Double

### Variables

Variables are declared with the syntax

```
var <VariableName> = <Expression>
```

### Functions

A basic function is declared with the `f` keyword.

```
f <FunctionName>(): <ReturnType> {
    // ...
}
``` 

Functions can have multiple return types

```
f <FunctionName>(): (<TypeName>, <TypeName>) {
    // ...
}
```

Functions can also be added to types

```
f <TypeName>.<FunctionName>(): <ReturnType> {
    // ... 
}
```

When a function is applied to a type, the state variables of that type can be accessed with `this`.

### Control Flow

Lila does not have `if` statements. The reason for this is that Lila was designed specifically for generating configuation files. Complicated control flow structures with multiple branches makes understanding the specific configuration of a system complicated. Instead, Lila provides three built-in types (Just, Maybe, and None) for handling control flow.

#### Just, Maybe, None

`Just` is a type that represents a single value. A `Just`'s output exactly matches its inner value. An instance of `Just` can be created with the built-in `just` function:

```
f just<T>(value: T): Just<T>
```

`None` is a type that represents nothing. When converted to output, anything with value `None` is always omitted.

```
f someFunc(): None
```

`Maybe` is a type that is either a `Just<T>` or a `None`. Maybe can be created by filtering a `Just`

```
f lessThanFive(value: Int): Maybe<Int> {
    return just(value).filter((v) -> { return v < 5 })
}
```

1. The output of maybe depends if it is `Just` or `None`. If a maybe is `Just`, then its output is the same as that `Just`. Otherwise, the `Maybe` does not produce any output. 
2. Filtering a `None` maybe always produces `None`.
3. An empty `Maybe<T>` can be converted to a `Just<T>` using `f Maybe<T>.or(default: T): Just<T>`.
4. A maybe can be converted to another value using `f Maybe<T>.transform<R>(mapper: (T) -> R, empty: () -> R): R`

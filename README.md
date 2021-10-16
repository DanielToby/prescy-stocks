## **A cross-platform live stock monitor with customizable, evaluated indicators**
![Alt Text](demo.gif)

### **Multithreaded live stock querying**
Stock data is pulled every ten seconds from the [Yahoo Finance API](https://finance.yahoo.com/quotes/API,Documentation/view/v1/). Eight threads are used to divvy up the work.

### **Indicators using Lua expression evaluation**
[Lua](https://www.lua.org/) expressions are evaluated after the queries are performed. Data is pushed onto the Lua stack in nested tables. When stock data is refreshed, the expression is evaluated. The result is shown to the right of each stock. So long as the syntax is correct and the necessary data is present, expressions can comprise any valid script. Here's an example for percent changed:

`setResult( (data[__size - 1].close - data[0].open) / data[__size - 1].close * 100 )`
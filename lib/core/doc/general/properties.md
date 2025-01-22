Qx Bindable Properties {#properties}
====================================

Bindable properties are properties that enable the establishment of relationships between various properties in a declarative manner. Properties with bindings, which are essentially just C++ functions, are updated automatically whenever one or more other properties that they depend on are changed. These dependencies are established automatically when a property is read during the binding evaluation of another.

The following is a prime example of their use:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
Qx::Property<int> width(2);
Qx::Property<int> height(2);
Qx::Property<int> area([&]{ return width * height; });
area.subscribeLifetime([&]{ qDebug() << "Area is:" << area; });
width = 3;
height = 1;

// Output
// Area is: 4
// Area is: 6
// Area is: 3
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The Qx Bindable Properties System can thought of as an alternate implementation of Qt Bindable Properties, and as such its interface is closely modeled after the latter.

The foundation of the system is Qx::AbstractBindableProperty, which represents the general bindable properties interface, while Qx::Property is the primary implementation of said interface. Additional utilities related to the system are accessible through the qx-property.h header.

Qx properties can, for the most part, be used interchangeably with Qt in the context of C++ code (QML integration is not supported, and may or may not be attempted at a later time), just with some behavioral and feature set quality of life changes; thus, for brevity this documentation focuses on the differences between the two systems and if you are totally unfamiliar with bindable properties it is recommended to read the documentation for Qt Bindable Properties first.

**The biggest difference between the two is that Qx Bindable Properties were designed with the motivation that bindings are only ever evaluated when absolutely necessary**, as there are various situations with Qt properties where extra binding evaluations occur.

What's the Same?
------------
Pretty much everything between Qx properties and Qt properties are the same, other than what is mentioned under the Advantages and Disadvantages sections below. Regardless, the following is a non-exhaustive list of some key aspects that both systems share that are important to keep in mind:

 - Most methods and the overall API is the same.
 - Qx::Bindable, like QBindable exists as a property wrapper that allows for generic access to any property that implements the bindable interface, and can wrap QObject properties (i.e. declared with Q_PROPERTY())
 - You can still group property value changes using Qx::beginPropertyUpdateGroup() and Qx::endPropertyUpdateGroup().
 - Dependency/update cycles are detected.
 - Qx properties are not thread safe. In general, only interact with a property through it's owning thread.
 - Qt's advice about writing intermediate values to properties and respective class invariants should still be respected
 - Observers (i.e. registered callback functions) are not notified of a property change until the entire update-chain in which the change occurred has finished resolving; that is, all dependent properties are first updated before any callbacks are invoked.

Advantages
------------
### Absolutely Minimal Binding Evaluation:

This is the largest advantage, and the main motivation for the creation of this system.

As impressive as the Qt Bindable Property system is, there is one aspect of it's behavior that can be frustrating and potentially problematic: It often re-evaluates bindings more times that would appear necessary, presumably due to technical limitations.

Let's take this simple example:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
QProperty<int> x;
QProperty<int> x2([&]{ return std::pow(x, 2.0); });
QProperty<int> poly([&]{ return x2 + x; });
auto n = poly.addNotifier([&]{ qDebug() << "Polynomial value:" << poly; }); 
x = 1;
x = 2;
x = 3;

// Output
// Polynomial value: 2
// Polynomial value: 6
// Polynomial value: 12
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Going off just the notifier callback output, nothing initially looks amiss.

Here we have a dependency graph that looks like this:
@cond@
This isn't as nice looking and has rendering issues with DoxygenAwesome (inverted colors and grey scrollbar)

@dot
digraph Dependencies {
    rankdir=BT;
    node [
        shape=circle,
        fixedsize=true,
        width=0.8,
        style=filled,
        color="#380000",
        fillcolor="#8F1717",
        
    ];
    splines=ortho;
    nodesep=1.5;
    ranksep=0.7;

    A [label="poly"];
    B [label="x2"];
    C [label="x"];
    
    B -> A;
    C -> A;
    C -> B;
}
@enddot
@endcond

![Polynomial property example graph](properties-0.png){html: width=35%}

Just at a glance we can see that when `x` is changed, `x2` should be updated before `poly` since the latter depends on both `x` and `x2`; however, if we change the example a little to gain some insight into how updates are handled, we see Qt Properties *do not do this*:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
qDebug() << "INITIAL SETUP";
QProperty<int> x;
QProperty<int> x2([&]{
    qDebug() << "eval x2, x is" << x.valueBypassingBindings();
    return std::pow(x, 2.0);
});
QProperty<int> poly([&]{
    qDebug() << "eval poly, x is" << x.valueBypassingBindings() << "x2 is" << x2.valueBypassingBindings();
    return x + x2;
});
auto n = poly.addNotifier([&]{ qDebug() << "Polynomial value:" << poly; });
qDebug() << "CHANGE START";
x = 1;
x = 2;
x = 3;

// Output
// INITIAL SETUP
// eval x2, x is 0
// eval poly, x is 0 x2 is 0
// CHANGE START
// eval poly, x is 1 x2 is 0
// eval x2, x is 1
// eval poly, x is 1 x2 is 1
// Polynomial value: 2
// eval poly, x is 2 x2 is 1
// eval x2, x is 2
// eval poly, x is 2 x2 is 4
// Polynomial value: 6
// eval poly, x is 3 x2 is 4
// eval x2, x is 3
// eval poly, x is 3 x2 is 9
// Polynomial value: 12
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

As shown, when we start updating `x`, the `poly` binding is evaluated first with a stale value of `x2`, then `x2` is updated, and finally `poly` is evaluated again. It's possible that declaration order, or some other details may influence this, but that is largely irrelevant, since ideally evaluation count should be consistent regardless of those factors. The takeaway is that the Qt Bindable Properties system does not *maximally* prioritize minimizing binding evaluations and instead only ensures that the final state of all properties is correct once its update process is finished, while keeping binding evaluations *somewhat* minimal. 

If we then simply change the use of QProperty in the above example to Qx::Property, the output becomes:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
INITIAL SETUP
eval x2, x is 0
eval poly, x is 0 x2 is 0
CHANGE START
eval x2, x is 1
eval poly, x is 1 x2 is 1
Polynomial value: 2
eval x2, x is 2
eval poly, x is 2 x2 is 4
Polynomial value: 6
eval x2, x is 3
eval poly, x is 3 x2 is 9
Polynomial value: 12
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

which shows that each involved binding is evaluated in a order that prevents any re-evaluations from being required.

At first, although obviously wasteful, it may not seem like a huge deal; however, consider the case where one of these properties might be checked to see if a particular resource is valid (like a pointer) and the other property wraps the resource itself. If the binding that uses both of these properties was evaluated with a stale value for the "resource is valid" property, it might then try to access an invalid resource and cause your program to crash.

Another benefit of Qx's approach is that it handles "incomplete dependency" information in bindings cleanly and has looser restrictions compared to Qt's in the sense that not all code paths need to read from all property dependency on every invocation. For example, if you originally had values you wanted to convert to properties that looked like this:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
// Goofy bools
bool round = false;
bool maybeBouncy = round;
bool ball = round && maybeBouncy;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

for the best experience with QProperty you're supposed to do:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
QProperty<bool> round (false);
QProperty<bool> maybeBouncy([&]{ return round.value(); });
QProperty<bool> ball([&]{
    bool r = round.value();
    bool mb = maybeBouncy.value();
    return r && mb;
});
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

so that both `round ` and `maybeBouncy` are always read within the binding and `ball`'s dependency on both is well-established.

With Qx::Property, you can simply do:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
Qx::Property<bool> round(false);
Qx::Property<bool> maybeBouncy([&]{ return round.value(); });
Qx::Property<bool> ball([&]{ return round.value() && maybeBouncy.value(); });
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When `round` is updated for the first time, it will appear like `ball` only depends on `round` since `maybeBouncy` wasn't read due to short-circuiting on the initial binding invocation during `ball`'s construction; however, Qx will handle this gracefully by temporarily "pausing" the evaluation of `ball`'s binding when it sees the dependency on `maybeBouncy` for the first time in order to ensure that property is updated first. Therefore, `ball` will not see a stale value for `maybeBouncy` and `ball`'s binding still only needs to run one time even though it's dependencies suddenly changed! 

Qx's implementation is designed so that bindings should **never** be evaluated more than absolutely necessary. If you've found a scenario in which this isn't true, please open an issue about it on GitHub.

### Other:

 - More idiomatic const correctness.
    - Due to implementation constraints, some methods on some Qt property classes that are "read" in nature (i.e. do not modify principle data) are non-const, making accessing them in non-const contexts impossible, even though you're not writing to the value in any way.
 - Use of `[[nodiscard]]` for callback handles to catch subtle bugs in which a callback would be immediately unregistered due to the handle being discarded
 - Non-templated callback handles.
	- All methods that return a callback function "handle" (i.e. lifecycle handler) return a non-templated type, which makes storing them in a container trivial, unlike `QPropertyChangeHandler<Functor>`
 - "Lifetime" callback registration.
    - Additional registration methods that tie the lifetime of the callback to the lifetime of the property and don't require the user to manage a handle object
 - More flexible operator->().
    - Implementation of operator->() that allows for chaining through more T types, like basic aggregates.

Disadvantages
------------
Given enough motivation, these drawbacks may be reduced or outright eliminated in the future.

 - No tie-in with QML, which is of course the biggest downside currently
 - No integration with QMetaObject/MOC
	 - These bindable properties cannot be queried, or used in a type-erased fashion through QMetaObject, since that would require modifying MOC itself
 - No advance error system like QPropertyBindingError that notes the source location of an error and allows reactionary steps to be taken at runtime.
	 - If something like a cycle is detected, the program simply aborts through an assertion with a diagnostic.
 - No equivalent to QObjectBindableProperty for making property data a "built-in" part of the QObject itself, though simply adding Qx::Property to a QObject derived class as a member variable is not much different
 - Performance between the two has not been properly profiled. It's conceivable that Qt properties are a bit more efficient given their maturity and pedigree, though the Qx implementation should still be fairly performant due to its approach. Additionally, the fact that Qt's system sometimes re-evaluates bindings when not necessary situationally gives an edge to Qx's system.
 - Some other minor facets of Qt Properties that Qx Properties do not have, which make the former a bit more robust
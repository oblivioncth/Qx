//! [0]
Qx::GenericError ge;
ge.setErrorLevel(Qx::GenericError::Warning);
ge.setCaption("Warning");
ge.setPrimaryInfo("Watch out for something!");
ge.setSecondaryInfo("Oops");
ge.setDetailedInfo("Problems");

Qx::postError(ge);
//! [0]
//! [0]
QString getTheKeysValue()
{
	QString keyValue = getMySetting("Key");

	if(keyValue.isNull())
	{
		return Qx::GenericError(Qx::GenericError::Error,
								"Failed to read settings file",
								"The target key 'Key' did not exist");
	}
}

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	QTextStream errStream(stderr); // For error printing
	
	Qx::GenericError keyReadError = getTheKeysValue();
	if(keyReadError.isValid())
	{
		errStream << keyReadError;
		app.exit(-1);
	}
	
	//...
	
	return app.exec();
}
//! [0]

//! [1]
Qx::GenericError ge;
ge.setErrorLevel(Qx::GenericError::Warning);
ge.setCaption("Caption");
ge.setPrimaryInfo("Generic Error");
ge.setSecondaryInfo("There was an Error");
ge.setDetailedInfo("- Issue 1\n- Issue2\n- Issue3");

QTextStream ts;
ts << ge;

// Prints:
/*
 *	WARNING: Caption 
 *  Generic Error
 *  There was an Error
 *
 *	Details:
 *  --------
 *  - Issue 1
 *  - Issue 2
 *  - Issue 3
 */
//! [1]
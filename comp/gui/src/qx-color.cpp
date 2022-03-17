// Unit Includes
#include "qx/gui/qx-color.h"

namespace Qx
{
	
//===============================================================================================================
// Color
//===============================================================================================================

/*!
 *  @class Color
 *
 *  @brief The Color class is a collection of static functions pertaining to colors.
 */

//-Class Functions-----------------------------------------------------------------------------------------------
//Public:

/*!
 *  Returns a pure black or pure white, whichever is more ideal for maximum visibility when displayed on top of
 *  @a bgColor.
 *
 *  The choice is calculated according to version 2.0 of the WC3 Web Content Accessibility Guidelines.
 */
QColor Color::textFromBackground(QColor bgColor)
{
    // Based on W3C recommendations, using black & white text
    // See: https://www.w3.org/TR/WCAG20/ and https://www.w3.org/TR/WCAG20/#relativeluminancedef
    double contrastThreshold = 0.179;
    std::function<double(double)> componentFunc = [](double ch) { return ch < 0.03928 ? ch/12.92 : std::pow((ch+0.055)/1.055, 2.4); };

    // Ensure color is using RGB
    if(bgColor.spec() != QColor::Rgb)
        bgColor = bgColor.toRgb();

    // Calculate Y709 luminance
    double Rc = componentFunc(bgColor.redF());
    double Gc = componentFunc(bgColor.greenF());
    double Bc = componentFunc(bgColor.blueF());

    double L = 0.2126 * Rc + 0.7152 * Gc + 0.0722 * Bc;

    // Return black or white text
    return L > contrastThreshold ? QColorConstants::Black : QColorConstants::White;
}

}

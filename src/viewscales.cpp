#include "view.h"

void
View::drawScales()
{
    // NOTE: currently disabled, since no PViews in mesher
    // std::vector<PView *> scales;
    // for(std::size_t i = 0; i < PView::list.size(); i++) {
    //     PViewData *data = PView::list[i]->getData();
    //     PViewOptions *opt = PView::list[i]->getOptions();
    //     if(!data->getDirty() && opt->visible && opt->showScale &&
    //         opt->type == PViewOptions::Plot3D && data->getNumElements() &&
    //         isVisible(PView::list[i]))
    //         scales.push_back(PView::list[i]);
    // }
    // if(scales.empty()) return;
    //
    // drawContext::global()->setFont(CTX::instance()->glFontEnum,
    //                                CTX::instance()->glFontSize);
    // char label[1024];
    // double maxw = 0.;
    // for(std::size_t i = 0; i < scales.size(); i++) {
    //     PViewOptions *opt = scales[i]->getOptions();
    //     snprintf(label, 1024, opt->format.c_str(), -M_PI * 1.e-4);
    //     maxw = std::max(maxw, drawContext::global()->getStringWidth(label));
    // }
    //
    // const double tic = 10., bar_size = 16.;
    // double width = 0., width_prev = 0., width_total = 0.;
    //
    // for(std::size_t i = 0; i < scales.size(); i++) {
    //     PView *p = scales[i];
    //     PViewData *data = p->getData();
    //     PViewOptions *opt = p->getOptions();
    //
    //     if(!opt->autoPosition) {
    //         double w = opt->size[0], h = opt->size[1];
    //         double x = opt->position[0], y = opt->position[1];
    //         int c = fix2dCoordinates(&x, &y);
    //         if(c & 1) x -= w / 2.;
    //         if(c & 2) y -= h / 2.;
    //         drawScale(this, p, x, y, w, h, tic,
    //                   CTX::instance()->post.horizontalScales);
    //     }
    //     else if(CTX::instance()->post.horizontalScales) {
    //         double ysep = 20.;
    //         double xc = (viewport[2] - viewport[0]) / 2.;
    //         if(scales.size() == 1) {
    //             double w = (viewport[2] - viewport[0]) / 2., h = bar_size;
    //             double x = xc - w / 2., y = viewport[1] + ysep;
    //             drawScale(this, p, x, y, w, h, tic, 1);
    //         }
    //         else {
    //             double xsep = maxw / 4. + (viewport[2] - viewport[0]) / 10.;
    //             double w = (viewport[2] - viewport[0] - 4 * xsep) / 2.;
    //             if(w < 20.) w = 20.;
    //             double h = bar_size;
    //             double x = xc - (i % 2 ? -xsep / 1.5 : w + xsep / 1.5);
    //             double y =
    //                 viewport[1] + ysep +
    //                 (i / 2) * (bar_size + tic +
    //                            2 * drawContext::global()->getStringHeight() + ysep);
    //             drawScale(this, p, x, y, w, h, tic, 1);
    //         }
    //     }
    //     else {
    //         double xsep = 20.;
    //         double dy = 2. * drawContext::global()->getStringHeight();
    //         if(scales.size() == 1) {
    //             double ysep = (viewport[3] - viewport[1]) / 6.;
    //             double w = bar_size, h = viewport[3] - viewport[1] - 2 * ysep - dy;
    //             double x = viewport[0] + xsep, y = viewport[1] + ysep + dy;
    //             drawScale(this, p, x, y, w, h, tic, 0);
    //         }
    //         else {
    //             double ysep = (viewport[3] - viewport[1]) / 15.;
    //             double w = bar_size;
    //             double h = (viewport[3] - viewport[1] - 3 * ysep - 2.5 * dy) / 2.;
    //             double x = viewport[0] + xsep + width_total + (i / 2) * xsep;
    //             double y =
    //                 viewport[1] + ysep + dy + (1 - i % 2) * (h + 1.5 * dy + ysep);
    //             drawScale(this, p, x, y, w, h, tic, 0);
    //         }
    //         // compute width
    //         width_prev = width;
    //         snprintf(label, 1024, opt->format.c_str(), -M_PI * 1.e-4);
    //         width = bar_size + tic + drawContext::global()->getStringWidth(label);
    //         if(opt->showTime) {
    //             char tmp[256];
    //             snprintf(tmp, 256, opt->format.c_str(), data->getTime(opt->timeStep));
    //             snprintf(label, 1024, "%s (%s)", data->getName().c_str(), tmp);
    //         }
    //         else
    //             snprintf(label, 1024, "%s", data->getName().c_str());
    //         width = std::max(width, drawContext::global()->getStringWidth(label));
    //         if(i % 2)
    //             width_total += std::max(bar_size + width, bar_size + width_prev);
    //     }
    // }
}

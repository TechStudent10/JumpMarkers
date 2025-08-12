#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

struct Jump {
    CCPoint location;
    int percent;
    bool isFlipped;
    bool useCircle;
};

class $modify(JMPlayLayer, PlayLayer) {
    struct Fields {
        CCLayer* deathLayer;
    };

    bool init(GJGameLevel *level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        m_fields->deathLayer = CCLayer::create();
        m_objectLayer->addChild(m_fields->deathLayer);

        return true;
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        resetDeathLayer();
    }

    void levelComplete() {
        PlayLayer::levelComplete();
        resetDeathLayer();
    }

    void togglePracticeMode(bool practiceMode) {
        PlayLayer::togglePracticeMode(practiceMode);
        resetDeathLayer();
    }

    void resetDeathLayer() {
        if (m_fields->deathLayer) m_fields->deathLayer->removeAllChildren();
    }
};

class $modify(JMPlayerObject, PlayerObject) {
    struct Fields {
        std::vector<Jump> jumps;
    };

    bool pushButton(PlayerButton btn) {
        if (!PlayLayer::get()) return PlayerObject::pushButton(btn);
        m_fields->jumps.push_back({
            .location = this->getPosition(),
            .percent = PlayLayer::get()->getCurrentPercentInt(),
            .isFlipped = m_isUpsideDown,
            .useCircle = m_isDart || m_isShip || m_isRobot
        });
        return PlayerObject::pushButton(btn);
    }

    void playerDestroyed(bool p0) {
        PlayerObject::playerDestroyed(p0);

        if (!PlayLayer::get()) return;

        for (auto point : m_fields->jumps) {
            auto rect = CCSprite::createWithSpriteFrameName("PBtn_Arrow_001.png");
            if (point.useCircle) {
                rect = CCSprite::create("circle.png");
            }
            rect->setBlendFunc({GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA});
            rect->setContentSize({ 25.f, 25.f });
            rect->setFlipY(point.isFlipped);
            rect->setScale(1.25f);

            float initPos = point.isFlipped ? 20.f : -20.f;
            rect->setPosition({
                point.location.x, point.location.y - initPos
            });
            rect->setOpacity(0);

            auto plFields = static_cast<JMPlayLayer*>(PlayLayer::get())->m_fields.self();
            plFields->deathLayer->addChild(rect);
            
            float duration = 1.f + (JMPlayLayer::get()->getCurrentPercent() - point.percent) / 100.0;
            rect->runAction(
                CCEaseExponentialOut::create(
                    CCFadeIn::create(duration)
                )
            );
            rect->runAction(
                CCEaseExponentialOut::create(
                    CCMoveBy::create(duration, { 0, initPos })
                )
            );
        }

        m_fields->jumps.clear();
    }
};

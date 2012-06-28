require File.dirname(__FILE__) + '/../test_helper'
require 'video_categories_controller'

# Re-raise errors caught by the controller.
class VideoCategoriesController; def rescue_action(e) raise e end; end

class VideoCategoriesControllerTest < Test::Unit::TestCase
  fixtures :video_categories

  def setup
    @controller = VideoCategoriesController.new
    @request    = ActionController::TestRequest.new
    @response   = ActionController::TestResponse.new

    @first_id = video_categories(:first).id
  end

  def test_index
    get :index
    assert_response :success
    assert_template 'list'
  end

  def test_list
    get :list

    assert_response :success
    assert_template 'list'

    assert_not_nil assigns(:video_categories)
  end

  def test_show
    get :show, :id => @first_id

    assert_response :success
    assert_template 'show'

    assert_not_nil assigns(:video_category)
    assert assigns(:video_category).valid?
  end

  def test_new
    get :new

    assert_response :success
    assert_template 'new'

    assert_not_nil assigns(:video_category)
  end

  def test_create
    num_video_categories = VideoCategory.count

    post :create, :video_category => {}

    assert_response :redirect
    assert_redirected_to :action => 'list'

    assert_equal num_video_categories + 1, VideoCategory.count
  end

  def test_edit
    get :edit, :id => @first_id

    assert_response :success
    assert_template 'edit'

    assert_not_nil assigns(:video_category)
    assert assigns(:video_category).valid?
  end

  def test_update
    post :update, :id => @first_id
    assert_response :redirect
    assert_redirected_to :action => 'show', :id => @first_id
  end

  def test_destroy
    assert_nothing_raised {
      VideoCategory.find(@first_id)
    }

    post :destroy, :id => @first_id
    assert_response :redirect
    assert_redirected_to :action => 'list'

    assert_raise(ActiveRecord::RecordNotFound) {
      VideoCategory.find(@first_id)
    }
  end
end
